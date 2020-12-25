<template>
	<div class="card">
		<ul class="list-gourp list-group-flush">
			<BoardArticleListItem
				v-for="article in articles"
				v-bind:key="[$route.params.boardname, '/', article.aid].join('')"
				v-bind:_boardname_en="$route.params.boardname"
				v-bind:_title="article.title"
				v-bind:_author="article.author"
				v-bind:_comments="article.th_num"
				v-bind:_aid="article.tid"
				/>
		</ul>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import { BOARD_ARTICLE_MODE } from "@/lib/BMYConstants.js"
import BoardArticleListItem from "@/components/BoardArticleListItem.vue"

export default {
	data() {
		return {
			articles: [],
		};
	},
	mounted() {
		BMYClient.get_article_list(this.$route.params.boardname, BOARD_ARTICLE_MODE.THREAD_MODE).then(response => {
			if (response.errcode == 0) {
				this.articles = response.articlelist.reverse();
			}
		});
	},
	components: {
		BoardArticleListItem,
	},
}
</script>

<style scoped>
</style>

