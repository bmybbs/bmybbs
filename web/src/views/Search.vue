<template>
	<div v-if="has_query">
		<div>搜索"{{ query }}"的结果</div>
		<ul v-if="result.length > 0">
			<li v-for="item in result" v-bind:key="item.aid"><router-link :to="{ name: 'thread', params: { boardname: $route.params.boardname, tid: item.tid }}" class="link">{{ item.title }}</router-link> / <fa icon="at" /><PopoverUserInfo :_userid="item.owner" /> 发表于 <TooltipTimestamp :_unix_timestamp="item.aid" /></li>
		</ul>
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
const PopoverUserInfo = defineAsyncComponent(() => import("@/components/PopoverUserInfo.vue"));

export default {
	data() {
		return {
			query: "",
			has_query: true,
			result: [],
			msg: "",
		};
	},
	created() {
		this.$watch(() => this.$route, (toRoute) => {
			if (toRoute.query.q != null) {
				this.query = toRoute.query.q.trim();
				this.doSearchContent();
			} else {
				this.has_query = false;
			}
		});
	},
	mounted() {
		if (this.$route.query.q != null) {
			this.query = this.$route.query.q.trim();
			this.doSearchContent();
		} else {
			this.has_query = false;
		}
	},
	methods: {
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
	},
	components: {
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

