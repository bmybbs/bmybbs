<template>
	<header class="navbar navbar-dark sticky-top bg-dark flex-md-nowrap p-0 shadow">
		<div class="navbar-brand col-md-3 col-lg-2 mr-0 px-3">BMYBBS</div>
		<button class="navbar-toggler position-absolute d-md-none collapsed" type="button" data-toggle="collapse" data-target="#sidebarMenu" aria-controls="sidebarMenu" aria-expanded="false" aria-label="Toggle navigation">
			<span class="navbar-toggler-icon"></span>
		</button>
		<div class="w-100">
			<NavSearch />
		</div>
	</header>

	<div class="container-fluid">
		<div class="row">
			<nav id="sidebarMenu" class="col-md-3 col-lg-2 d-md-block bg-light sidebar collapse">
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
</template>

<script>
import { BMYSECSTRS } from "@/lib/BMYConstants.js";
import SidebarSecList from "@/components/SidebarSecList.vue";
import NavSearch from "@/components/NavSearch.vue";

export default {
	data() {
		return {
			sections: BMYSECSTRS,
		}
	},
	mounted() {
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
}

.navbar .navbar-toggler {
	top: .25rem;
	right: 1rem;
}

/* Main */
main {
	padding-top: 16px;
}
</style>

