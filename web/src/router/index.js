import { createWebHistory, createRouter } from "vue-router";
import Home from "@/views/Home.vue"
import Web from "@/views/Web.vue"
import Dashboard from "@/views/Dashboard.vue"

const routes = [
	{
		path: "/",
		name: "Home",
		component: Home,
	},
	{
		path: "/web",
		component: Web,
		children: [
			{
				path: "",
				component: Dashboard
			},
		]
	},
];

const router = createRouter({
	history: createWebHistory(),
	routes,
});

export default router;

