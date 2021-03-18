<template>
	<div>
		<ArticleViewer v-for="article in articlelist"
			v-bind:key="article.aid"
			v-bind:_boardname_en="$route.params.boardname"
			v-bind:_aid="article.aid"
			v-bind:_mark="article.mark"
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
			thread_title: "",
			board: "",
			articlelist: [],
		};
	},
	mounted() {
		BMYClient.get_thread_list(this.$route.params.boardname, this.$route.params.tid).then(response => {
			if (response.errcode == 0) {
				this.articlelist = response.articlelist;
				if (Array.isArray(response.articlelist) && response.articlelist.length > 0) {
					this.thread_title = response.articlelist[0].title;
					this.board = response.articlelist[0].board;
				}
			}
		});
	},
	components: {
		ArticleViewer,
	},
}
</script>

<style scoped>
</style>

