<template>
	<div v-if="has_query">
		<div v-if="type == 2"><!-- content -->
			<div>搜索"{{ query }}"的结果</div>
			<ul v-if="result.length > 0">
				<li v-for="item in result" v-bind:key="item.aid"><router-link :to="{ name: 'thread', params: { boardname: $route.params.boardname, tid: item.tid }}" class="link">{{ item.title }}</router-link> / <fa icon="at" /><PopoverUserInfo :_userid="item.owner" /> 发表于 <TooltipTimestamp :_unix_timestamp="item.aid" /></li>
			</ul>
		</div>
		<div v-if="type == 1"><!-- user -->
			<div>名称包含"{{ query }}"的用户有</div>
			<ul v-if="users.length > 0">
				<li v-for="user in users" v-bind:key="user"><PopoverUserInfo :_userid="user" /></li>
			</ul>
		</div>
		<div v-if="type == 0"><!-- board -->
			<div>名称包含"{{ query }}"的版面有</div>
			<ul v-if="boards.length > 0">
				<li v-for="board in boards" v-bind:key="board.name"><PopoverBoardInfo :_boardname_zh="board.name" :_boardname_en="board.name" /></li>
			</ul>
		</div>
		<div v-if="msg.length > 0">{{ msg }}</div>
	</div>
	<div v-else>
		您并没有输入想要搜索的内容哦...
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
import { BMYClient } from "@/lib/BMYClient.js"
import { BMY_EC } from "@/lib/BMYConstants.js"
import { getErrorMessage } from "@/lib/BMYUtils.js"

const TooltipTimestamp = defineAsyncComponent(() => import("@/components/TooltipTimestamp.vue"));
const PopoverBoardInfo = defineAsyncComponent(() => import("@/components/PopoverBoardInfo.vue"));
const PopoverUserInfo = defineAsyncComponent(() => import("@/components/PopoverUserInfo.vue"));

const SEARCH_TYPE = {
	BOARD:   0, /* board search */
	USER:    1, /* user search */
	CONTENT: 2, /* content search */
};

export default {
	data() {
		return {
			query: "",
			has_query: true,
			result: [],
			boards: [],
			users:  [],
			msg: "",
			type: -1,
		};
	},
	created() {
		this.$watch(() => this.$route, (toRoute) => {
			this.init(toRoute);
		});
	},
	mounted() {
		this.init(this.$route);
	},
	methods: {
		init(route) {
			if (route.query.q != null) {
				this.query = route.query.q.trim();
			} else {
				this.has_query = false;
			}

			if (route.name == "contentSearch") {
				this.type = SEARCH_TYPE.CONTENT;
				this.doSearchContent();
			} else if (route.name == "search") {
				if (route.query.type != null) {
					if (route.query.type == "user") {
						this.type = SEARCH_TYPE.USER;
						this.doSearchUser();
					} else {
						this.type = SEARCH_TYPE.BOARD;
						this.doSearchBoard();
					}
				}
			}
		},
		doSearchBoard() {
			BMYClient.search_board(this.query).then(response => {
				if (response.errcode == 0 && Array.isArray(response.board_array)) {
					this.boards = response.board_array;
				} else {
					this.boards.length = 0;
				}
			});
		},
		doSearchContent() {
			if (this.query.length > 0) {
				BMYClient.search_content(this.$route.params.boardname, this.query).then(response => {
					if (response.errcode == BMY_EC.API_RT_SUCCESSFUL) {
						this.result = response.result;
					} else {
						this.msg = getErrorMessage(response.errcode);
					}
				});
			} else {
				this.has_query = false;
			}
		},
		doSearchUser() {
			BMYClient.search_user(this.query).then(response => {
				if (response.errcode == 0 && Array.isArray(response.user_array)) {
					this.users = response.user_array;
				} else {
					this.users.length = 0;
				}
			});
		},
	},
	components: {
		PopoverBoardInfo,
		PopoverUserInfo,
		TooltipTimestamp,
	},
}
</script>

<style scoped>
.link {
	text-decoration: none;
}
</style>

