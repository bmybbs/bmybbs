<template>
	<div class="row">
		<div class="col-md-9">
			<div class="card">
				<ul class="list-gourp list-group-flush">
					<BoardArticleListItem
						v-for="article in articles"
						v-bind:key="[$route.params.boardname, '/', article.aid].join('')"
						v-bind:_title="article.title"
						v-bind:_author="article.author"
						v-bind:_comments="article.th_num"
						v-bind:_aid="article.tid"
					/>
				</ul>
			</div>
		</div>
		<div class="col-md-3" v-if="info">
			<CardBoardInfo
				v-bind:_boardname_en="$route.params.boardname"
				v-bind:_boardname_zh="info.zh_name"
				v-bind:_secstr="info.secstr"
				v-bind:_article_num="info.article_num"
				v-bind:_thread_num="info.thread_num"
				v-bind:_inboard_num="info.inboard_num"
				v-bind:_today_new="info.today_new"
			/>
		</div>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import { BOARD_ARTICLE_MODE } from "@/lib/BMYConstants.js"
import BoardArticleListItem from "@/components/BoardArticleListItem.vue"
import CardBoardInfo from "@/components/CardBoardInfo.vue"

export default {
	data() {
		return {
			info: null,
			articles: [],
		};
	},
	mounted() {
		BMYClient.get_article_list(this.$route.params.boardname, BOARD_ARTICLE_MODE.THREAD_MODE).then(response => {
			if (response.errcode == 0) {
				this.articles = response.articlelist.reverse();
			}
		});

		BMYClient.get_board_info(this.$route.params.boardname).then(response => {
			this.info = response;
		});
	},
	components: {
		BoardArticleListItem,
		CardBoardInfo,
	},
}
</script>

<style scoped>
</style>

