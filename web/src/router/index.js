import { createWebHistory, createRouter } from "vue-router";
import Home from "@/views/Home.vue"
import Web from "@/views/Web.vue"
import Dashboard from "@/views/Dashboard.vue"
import Follow from "@/views/Follow.vue"
import Feed from "@/views/Feed.vue"
import Board from "@/views/Board.vue"
import Thread from "@/views/Thread.vue"

const routes = [{
	path: "/",
	name: "Home",
	component: Home,
}, {
	path: "/web",
	component: Web,
	children: [{
		path: "",
		component: Dashboard
	}, {
		path: "/web/feed",
		component: Feed
	}, {
		path: "/web/follow",
		component: Follow
	}, {
		path: "/web/board/:boardname",
		component: Board
	}, {
		path: "/web/board/:boardname/thread/:tid",
		name: "thread",
		component: Thread
	}]
}];

const router = createRouter({
	history: createWebHistory(),
	routes,
});

export default router;

