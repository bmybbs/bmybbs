import { createWebHistory, createRouter } from "vue-router";

const routes = [{
	path: "/",
	name: "Home",
	component: () => import("@/views/Home.vue"),
}, {
	path: "/web",
	component: () => import("@/views/Web.vue"),
	children: [{
		path: "",
		component: () => import("@/views/Dashboard.vue"),
	}, {
		path: "/web/feed",
		name: "feed",
		component: () => import("@/views/Feed.vue"),
	}, {
		path: "/web/follow",
		component: () => import("@/views/Follow.vue"),
	}, {
		path: "/web/section",
		component: () => import("@/views/Section.vue"),
	}, {
		path: "/web/section/:secid",
		name: "section",
		component: () => import("@/views/Feed.vue"),
	}, {
		path: "/web/board/:boardname",
		component: () => import("@/views/Board.vue"),
		children: [{
			path: "",
			name: "board",
			component: () => import("@/views/BoardView.vue"),
		}, {
			path: "/web/board/:boardname/submit",
			component: () => import("@/views/Submit.vue"),
			name: "boardSubmit",
		}, {
			path: "/web/board/:boardname/thread/:tid",
			name: "thread",
			component: () => import("@/views/Thread.vue"),
		}, {
			path: "/web/board/:boardname/reply/:aid",
			name: "reply",
			component: () => import("@/views/Reply.vue"),
		}]
	}, {
		path: "/web/settings",
		component: () => import("@/views/Settings.vue"),
	}, {
		path: "/web/submit",
		component: () => import("@/views/SubmitWrapper.vue"),
		children: [{
			path: "",
			component: () => import("@/views/Submit.vue"),
			name: "RAWSUBMIT",
		}]
	}]
}];

const router = createRouter({
	history: createWebHistory(),
	routes,
});

export default router;

