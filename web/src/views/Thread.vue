<template>
	<div>
		<ArticleViewer v-for="article in articlelist"
			v-bind:key="article.aid"
			v-bind:_boardname_en="$route.params.boardname"
			v-bind:_aid="article.aid"
			v-bind:_mark="article.mark"
			v-bind:_unread="article.unread"
			v-bind:_reply_callback="updateList"
		/>
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
import { BMYClient } from "@/lib/BMYClient.js"

const ArticleViewer = defineAsyncComponent(() => import("@/components/ArticleViewer.vue"));

export default {
	data() {
		return {
			articlelist: [],
		};
	},
	mounted() {
		this.updateList();
	},
	methods: {
		updateList() {
			BMYClient.get_thread_list(this.$route.params.boardname, this.$route.params.tid).then(response => {
				if (response.errcode == 0) {
					this.articlelist = response.articlelist;
				}
			});
		},
	},
	components: {
		ArticleViewer,
	},
}
</script>

<style scoped>
</style>

