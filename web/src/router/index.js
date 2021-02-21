import { createWebHistory, createRouter } from "vue-router";
import Home from "@/views/Home.vue"
import Web from "@/views/Web.vue"
import Dashboard from "@/views/Dashboard.vue"
import Follow from "@/views/Follow.vue"
import Feed from "@/views/Feed.vue"
import Board from "@/views/Board.vue"
import BoardView from "@/views/BoardView.vue"
import Thread from "@/views/Thread.vue"
import Settings from "@/views/Settings.vue"
import Section from "@/views/Section.vue"
import Submit from "@/views/Submit.vue"
import SubmitWrapper from "@/views/SubmitWrapper.vue"

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
		name: "feed",
		component: Feed
	}, {
		path: "/web/follow",
		component: Follow
	}, {
		path: "/web/section",
		component: Section
	}, {
		path: "/web/section/:secid",
		name: "section",
		component: Feed
	}, {
		path: "/web/board/:boardname",
		component: Board,
		children: [{
			path: "",
			name: "board",
			component: BoardView
		}, {
			path: "/web/board/:boardname/submit",
			component: Submit,
			name: "boardSubmit",
		}, {
			path: "/web/board/:boardname/thread/:tid",
			name: "thread",
			component: Thread
		}]
	}, {
		path: "/web/settings",
		component: Settings
	}, {
		path: "/web/submit",
		component: SubmitWrapper,
		children: [{
			path: "",
			component: Submit,
			name: "RAWSUBMIT",
		}]
	}]
}];

const router = createRouter({
	history: createWebHistory(),
	routes,
});

export default router;

