<template>
	<div>
		<Pagination :_current="page" :_total="total" :_callback="gotoPage" />
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
					v-bind:_unread="article.unread"
				/>
			</ul>
		</div>
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
import { BMYClient } from "@/lib/BMYClient.js"
import {
	BOARD_ARTICLE_MODE,
	ITEMS_PER_PAGE,
} from "@/lib/BMYConstants.js"
const BoardArticleListItem = defineAsyncComponent(() => import("@/components/BoardArticleListItem.vue"));
const Pagination = defineAsyncComponent(() => import("@/components/Pagination.vue"));

export default {
	data() {
		return {
			articles: [],
			total: 0,
			page: 1,
		};
	},
	created() {
		this.$watch(() => this.$route, (toRoute) => {
			if (toRoute.query && toRoute.query.page) {
				this.page = toRoute.query.page;
				this.get_list(toRoute.params.boardname, BOARD_ARTICLE_MODE.THREAD_MODE, toRoute.query.page);
			} else {
				this.get_list(toRoute.params.boardname, BOARD_ARTICLE_MODE.THREAD_MODE, 1);
				this.page = 1;
			}
		});
	},
	mounted() {
		this.page = (this.$route.query && this.$route.query.page) ? this.$route.query.page : 1;
		this.get_list(this.$route.params.boardname, BOARD_ARTICLE_MODE.THREAD_MODE, this.page);
	},
	methods: {
		get_list(boardname, mode, page) {
			BMYClient.get_article_list_by_board(boardname, mode, page).then(response => {
				if (response.errcode == 0) {
					this.articles = response.articlelist.reverse();
					this.total = Math.ceil(response.total / ITEMS_PER_PAGE);
				}
			});
		},
		gotoPage(pagenumber) {
			const r = {
				name: "board",
				params: this.$route.params
			};

			if (pagenumber > 1) {
				r.query = { page: pagenumber };
			}

			this.$router.push(r);
		},
	},
	components: {
		BoardArticleListItem,
		Pagination,
	},
}
</script>

<style scoped>
</style>

