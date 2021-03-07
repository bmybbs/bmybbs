<template>
	<header class="row sticky-top bg-dark m-0">
		<nav class="navbar navbar-dark p-0">
			<div class="navbar-brand col-3 col-sm-3 col-md-3 col-lg-2 mr-0 px-3">BMYBBS</div>
			<div class="col-6 col-sm-7 col-md-8 col-lg-9">
				<NavSearch />
			</div>
			<div class="col-3 col-sm-2 col-md-1 col-lg-1 px-3">
				<div class="dropdown">
					<button class="navbar-toggler dropdown-toggle" ref="dropdownUser" id="dropdownUserButton" type="button" data-bs-toggle="dropdown1" @click="toggle_ddu">
						<fa icon="user" />
						<span class="position-absolute top-0 start-50 translate-middle badge rounded-pill bg-danger" v-if="counted_messages > 0">{{counted_messages}}<span class="visually-hidden">未读消息</span></span>
					</button>
					<ul class="dropdown-menu dropdown-menu-end" :class="{ show: show_ddu }" aria-labelledby="dropdownUserButton" v-if="login_ok">
						<li>
							<div class="dropdown-item">
								站内信
								<span class="float-end badge rounded-pill bg-danger" v-if="user_info.unread_mail > 0">{{user_info.unread_mail}}</span>
							</div>
						</li>
						<li>
							<div class="dropdown-item">
								提醒
								<span class="float-end badge rounded-pill bg-danger" v-if="user_info.unread_notify > 0">{{user_info.unread_notify}}</span>
							</div>
						</li>
						<li><hr class="dropdown-divider"></li>
						<li><span class="dropdown-item" @click="logout">登出 {{user_info.userid}}</span></li>
					</ul>
					<ul class="dropdown-menu dropdown-menu-end" :class="{ show: show_ddu }" aria-labelledby="dropdownUserButton" v-else>
						<li><span class="dropdown-item" @click="gotoLogin">登录</span></li>
					</ul>
				</div>
			</div>
		</nav>
	</header>

	<div class="container-fluid">
		<div class="row">
			<nav id="sidebarMenu" class="col-md-3 col-lg-2 d-md-block d-none sidebar">
				<div class="sidebar-sticky pt-3 overflow-auto">
					<ul class="nav flex-column">
						<li class="nav-item">
							<router-link to="/web" class="nav-link">
								<span class="sidebar-icon"><fa icon="chart-line" /></span> 导读
							</router-link>
						</li>
						<li class="nav-item">
							<router-link to="/web/feed" class="nav-link">
								<span class="sidebar-icon"><fa icon="rss" /></span> 订阅
							</router-link>
						</li>
						<li class="nav-item">
							<router-link :to="($route.name == 'RAWSUBMIT' || $route.name == 'boardSubmit') ? '#' : (($route.name == 'board' || $route.name == 'thread' ) ? { name: 'boardSubmit', params: { boardname: $route.params.boardname }} : { name: 'RAWSUBMIT' })" class="nav-link">
								<span class="sidebar-icon"><fa icon="plus" /></span> 发布
							</router-link>
						</li>
						<li class="nav-item">
							<router-link to="/web/follow" class="nav-link">
								<span class="sidebar-icon"><fa icon="share-alt" /></span> 好友
							</router-link >
						</li>
						<li class="nav-item">
							<router-link to="/web/settings" class="nav-link">
								<span class="sidebar-icon"><fa icon="sliders-h" /></span> 设置
							</router-link>
						</li>
					</ul>

					<h6 class="sidebar-heading d-flex justify-content-between align-items-center px-3 mt-4 mb-1 text-muted">
						<span>分类讨论区</span>
					</h6>
					<div class="accordion">
						<SidebarSecList v-for="section in sections" v-bind:key="section.id" v-bind:_name="section.name" v-bind:_sec_id="section.id" v-bind:_icon="section.icon" />
					</div>
				</div>
			</nav>
			<main class="col-md-9 ms-sm-auto col-lg-10 px-md-4 min-vh-100">
				<router-view />
			</main>
		</div>
	</div>

	<footer class="d-md-none fixed-bottom">
		<nav class="navbar navbar-dark bg-dark">
			<ul class="navbar-nav w-100 flex-row justify-content-around">
				<li class="nav-item">
					<router-link to="/web" class="nav-link">
						<span class="sidebar-icon"><fa icon="chart-line" /></span> 导读
					</router-link>
				</li>
				<li class="nav-item">
					<router-link to="/web/feed" class="nav-link">
						<span class="sidebar-icon"><fa icon="rss" /></span> 订阅
					</router-link>
				</li>
				<li class="nav-item">
					<router-link to="/web/section" class="nav-link">
						<span class="sidebar-icon"><fa icon="layer-group" /></span> 分区
					</router-link >
				</li>
				<li class="nav-item">
					<router-link to="/web/settings" class="nav-link">
						<span class="sidebar-icon"><fa icon="sliders-h" /></span> 设置
					</router-link>
				</li>
			</ul>
		</nav>
	</footer>
</template>

<script>
import { BMYSECSTRS, BMY_EC } from "@/lib/BMYConstants.js";
import { BMYClient } from "@/lib/BMYClient.js";
import SidebarSecList from "@/components/SidebarSecList.vue";
import NavSearch from "@/components/NavSearch.vue";

export default {
	data() {
		return {
			load_user_meta_interval: null,
			user_info: {},
			show_ddu: false,
			login_ok: false,
			sections: BMYSECSTRS,
		}
	},
	computed: {
		counted_messages() {
			return this.user_info.unread_mail + this.user_info.unread_notify;
		},
	},
	mounted() {
		this.load_user_meta();
		this.load_user_meta_interval = setInterval(this.load_user_meta, 60 * 1000); // 60s
	},
	beforeUnmount() {
		if (this.load_user_meta_interval) {
			clearInterval(this.load_user_meta_interval);
			this.load_user_meta_interval = null;
		}
	},
	methods: {
		toggle_ddu() {
			this.show_ddu = !this.show_ddu;
		},
		logout() {
			BMYClient.user_logout().then(() => {
				this.gotoLogin();
			});
		},
		gotoLogin() {
			this.$router.push("/");
		},
		load_user_meta() {
			BMYClient.get_user_info("").then(response => {
				if (response.errcode != BMY_EC.API_RT_SUCCESSFUL) {
					this.login_ok = false;
					return;
				}

				this.login_ok = true;
				this.user_info = response;
			});
		},
	},
	components: {
		NavSearch,
		SidebarSecList
	},
}
</script>

<style scoped>
/* Sidebar */
.sidebar {
	position: fixed;
	top: 0;
	bottom: 0;
	left: 0;
	z-index: 100; /* Behind the navbar */
	padding: 48px 0 0; /* Height of navbar */
	box-shadow: inset -1px 0 0 rgba(0, 0, 0, .1);
}

@media (max-width: 767.98px) {
	.sidebar {
		top: 5rem;
	}
}

.sidebar-sticky {
	position: relative;
	top: 0;
	height: calc(100vh - 48px);
	padding-top: .5rem;
	background-color: #f5f9ff;
}

.sidebar-icon {
	display: inline-block;
	width: 16px;
	height: 16px;
	color: #6c757daa;
}

/* Navbar */
.navbar-brand {
	padding: .75rem 0;
	font-size: 1rem;
	background-color: rgba(0, 0, 0, 0, .25);
	box-shadow: inset -1px 0 0 rgba(0, 0, 0, 0, .25);
	margin-right: 0;
}

.navbar .navbar-toggler {
	top: .25rem;
	right: 1rem;
}

footer .navbar-nav .nav-link {
	padding: 0.5rem 1rem;
}

/* Main */
main {
	padding-top: 16px;
	padding-bottom: 50px;
}
</style>

