<template>
	<div class="card border-bmy-blue border-start-0 border-end-0">
		<ul class="list-group list-group-flush">
			<BoardArticleListItem
				v-for="article in articles"
				v-bind:key="[$route.params.boardname, '/', article.aid].join('')"
				v-bind:_boardname_en="$route.params.boardname"
				v-bind:_title="article.title"
				v-bind:_author="article.author"
				v-bind:_comments="article.th_num"
				v-bind:_aid="article.tid"
				v-bind:_accessed="article.mark"
				/>
		</ul>
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
import { BMYClient } from "@/lib/BMYClient.js"
import { BOARD_ARTICLE_MODE } from "@/lib/BMYConstants.js"
const BoardArticleListItem = defineAsyncComponent(() => import("@/components/BoardArticleListItem.vue"));

export default {
	data() {
		return {
			articles: [],
		};
	},
	created() {
		this.$watch(() => this.$route.params, (toParams) => {
			this.get_list(toParams.boardname, BOARD_ARTICLE_MODE.THREAD_MODE);
		});
	},
	mounted() {
		this.get_list(this.$route.params.boardname, BOARD_ARTICLE_MODE.THREAD_MODE);
	},
	methods: {
		get_list(boardname, mode) {
			BMYClient.get_article_list_by_board(boardname, mode).then(response => {
				if (response.errcode == 0) {
					this.articles = response.articlelist.reverse();
				}
			});
		},
	},
	components: {
		BoardArticleListItem,
	},
}
</script>

<style scoped>
</style>

