<template>
	<header class="navbar navbar-dark sticky-top bg-dark flex-md-nowrap p-0 shadow">
		<div class="navbar-brand col-md-3 col-lg-2 mr-0 px-3">BMYBBS</div>
		<button class="navbar-toggler position-absolute d-md-none collapsed" type="button" data-toggle="collapse" data-target="#sidebarMenu" aria-controls="sidebarMenu" aria-expanded="false" aria-label="Toggle navigation">
			<span class="navbar-toggler-icon"></span>
		</button>
	</header>

	<div class="container-fluid">
		<div class="row">
			<nav id="sidebarMenu" class="col-md-3 col-lg-2 d-md-block bg-light sidebar collapse">
				<div class="position-sticky pt-3">
					<ul class="nav flex-column">
					</ul>
				</div>
			</nav>
			<main class="col-md-9 ml-sm-auto col-lg-10 px-md-4">
				<router-view />
			</main>
		</div>
	</div>
</template>

<style scoped>
/*
 * Sidebar
 */
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

/*
 * Navbar
 */
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

/*
 * Main
 */
main {
	padding-top: 16px;
}
</style>

