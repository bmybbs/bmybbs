<template>
	<header class="row sticky-top bg-dark m-0 shadow">
		<nav class="navbar navbar-dark">
			<div class="navbar-brand col-3 col-sm-3 col-md-3 col-lg-2 mr-0 px-3">BMYBBS</div>
			<div class="col-6 col-sm-7 col-md-8 col-lg-9">
				<NavSearch />
			</div>
			<div class="col-3 col-sm-2 col-md-1 col-lg-1 px-3">
				<div class="dropdown">
					<button class="navbar-toggler dropdown-toggle" ref="dropdownUser" id="dropdownUserButton" type="button" data-bs-toggle="dropdown1" @click="toggle_ddu">
						<fa icon="user" />
					</button>
					<ul class="dropdown-menu dropdown-menu-end" :class="{ show: show_ddu }" aria-labelledby="dropdownUserButton">
						<li v-if="!loaded_ddu">
							<div class="d-flex justify-content-center">
								<div class="spinner-border text-secondary m-5" role="status">
									<span class="visually-hidden">Loading...</span>
								</div>
							</div>
						</li>

						<li v-if="loaded_ddu && login_ok"><span class="dropdown-item" @click="logout">登出 {{userid}}</span></li>
						<li v-if="loaded_ddu && !login_ok"><span class="dropdown-item" @click="gotoLogin">登录</span></li>
					</ul>
				</div>
			</div>
		</nav>
	</header>

	<div class="container-fluid">
		<div class="row">
			<nav id="sidebarMenu" class="col-md-3 col-lg-2 d-md-block d-none bg-light sidebar">
				<div class="sidebar-sticky pt-3">
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
			<main class="col-md-9 ms-sm-auto col-lg-10 px-md-4">
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
		</nav>
	</footer>
</template>

<script>
import { BMYSECSTRS } from "@/lib/BMYConstants.js";
import { BMYClient } from "@/lib/BMYClient.js";
import SidebarSecList from "@/components/SidebarSecList.vue";
import NavSearch from "@/components/NavSearch.vue";

export default {
	data() {
		return {
			show_ddu: false,
			loaded_ddu: false,
			login_ok: false,
			userid: "",
			sections: BMYSECSTRS,
		}
	},
	mounted() {
	},
	methods: {
		toggle_ddu() {
			this.show_ddu = !this.show_ddu;
			this.loaded_ddu = false;
			BMYClient.user_check().then(response => {
				this.loaded_ddu = true;

				if (response.code == 0) {
					this.login_ok = true;
					this.userid = response.userid;
				}
			});
		},
		logout() {
			BMYClient.user_logout().then(() => {
				this.gotoLogin();
			});
		},
		gotoLogin() {
			this.$router.push("/");
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
	overflow-x: hidden;
	overflow-y: auto; /* Scrollable contents if viewport is shorter than content. */
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
}
</style>

