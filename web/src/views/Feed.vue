<template>
	<div v-if="loggedin">
		<div class="row px-4">
			<div class="col-6 col-xs-6 col-md-6 px-1">
				<h2 v-if="feedmode">订阅</h2>
				<h2 v-else>{{$route.params.secid}}区</h2>
			</div>
			<div class="col-6 col-xs-6 col-md-6 right-col d-flex align-items-center justify-content-end">
				<span class="icon me-3">
					<fa icon="align-justify" />
				</span>
				<span class="icon">
					<fa icon="redo" />
				</span>
			</div>
		</div>
		<div class="row">
			<div class="col-md-9">
				<Pagination :_current="page" :_total="total" :_callback="gotoPage" />
				<div class="card border-bmy-blue border-start-0 border-end-0">
					<ul class="list-group list-group-flush">
						<DashboardArticleListItem
							v-for="article in articles"
							v-bind:key="[article.boardname_en, '/', article.aid].join('')"
							v-bind:_boardname_zh="article.boardname_zh"
							v-bind:_boardname_en="article.boardname_en"
							v-bind:_title="article.title"
							v-bind:_author="article.author"
							v-bind:_comments="article.count"
							v-bind:_aid="article.tid"
							v-bind:_accessed="article.accessed"
						/>
					</ul>
					<div class="card-footer text-center" @click="load_more">
						加载更多
					</div>
				</div>
			</div>
			<div class="col-md-3">
				<div class="card border-bmy-blue1">
					<div v-if="feedmode" class="card-header bg-bmy-blue1 bg-gradient text-white">
						收藏夹列表
					</div>
					<div v-else class="card-header bg-bmy-blue1 bg-gradient text-white">
						{{$route.params.secid}}区版面列表
					</div>
					<ul class="list-group list-group-flush">
						<li class="list-group-item"
							v-for="board in favboards"
							v-bind:key="board.name"><PopoverBoardInfo :_boardname_zh="board.name" :_boardname_en="board.name" /></li>
					</ul>
				</div>
			</div>
		</div>
	</div>
	<div v-else>
		请先<router-link to="/" class="text-decoration-none">登录</router-link>
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
import { BMYClient } from "@/lib/BMYClient.js"
import { BOARD_SORT_MODE, BMY_EC } from "@/lib/BMYConstants.js"

const DashboardArticleListItem = defineAsyncComponent(() => import("@/components/DashboardArticleListItem.vue"));
const PopoverBoardInfo = defineAsyncComponent(() => import("@/components/PopoverBoardInfo.vue"));
const Pagination = defineAsyncComponent(() => import("@/components/Pagination.vue"));

export default {
	data() {
		return {
			feedmode: this.$route.name == "feed",
			time: Math.floor(new Date().getTime() / 1000),
			articles: [ ],
			favboards: [ ],
			loggedin: true,
			total: 1,
			page: 1,
		}
	},
	created() {
		this.$watch(() => this.$route, (toRoute) => {
			this.feedmode = toRoute.name == "feed";
			this.time = Math.floor(new Date().getTime() / 1000);
			this.articles = [ ];
			this.favboards = [ ];

			this.init_data();
		});
	},
	mounted() {
		this.init_data();
	},
	methods: {
		init_data() {
			if (this.feedmode) {
				this.load_feed_more();
				this.load_favboards();
			} else {
				this.load_section_more();
				this.load_section_boards();
			}
		},
		gotoPage(pagenumber) {
			const r = {
				name: this.$route.name,
				params: this.$route.params
			};

			if (pagenumber > 1) {
				r.query = { page: pagenumber };
			}

			this.$router.push(r);
		},
		load_more() {
			if (this.feedmode) {
				this.load_feed_more();
			} else {
				this.load_section_more();
			}
		},
		load_feed_more() {
			this.page = this.$route.query.page ? this.$route.query.page : 1;
			BMYClient.get_feed(this.page).then(response => {
				switch (response.errcode) {
				case BMY_EC.API_RT_SUCCESSFUL:
					if (Array.isArray(response.articles)) {
						this.articles = this.articles.concat(response.articles);
						let l = response.articles.length;
						this.time = response.articles[l - 1].tid - 1;
					}
					this.total = response.total;
					break;
				case BMY_EC.API_RT_NOTLOGGEDIN:
					this.loggedin = false;
					break;
				case BMY_EC.API_RT_NOMOREFEED:
					this.$toast.info("没有更多内容", {
						position: "top"
					});
					break;
				}
			});
		},
		load_favboards() {
			BMYClient.get_fav_board_list().then(response => {
				if (response.errcode == 0 && Array.isArray(response.board_array)) {
					this.favboards = response.board_array;
				}
			});
		},
		load_section_boards() {
			BMYClient.get_boards_by_section(this.$route.params.secid, BOARD_SORT_MODE.BY_ALPHABET).then(response => {
				if (response.errcode == 0) {
					this.favboards = response.boardlist;
				}
			});
		},
		load_section_more() {
			this.page = this.$route.query.page ? this.$route.query.page : 1;
			BMYClient.get_article_list_by_section(this.$route.params.secid, this.page).then(response => {
				if (Array.isArray(response.articles)) {
					this.articles = response.articles;
				}
				this.total = response.total;
			});
		},
	},
	components: {
		DashboardArticleListItem,
		Pagination,
		PopoverBoardInfo,
	}
}
</script>

<style scoped>
span.icon {
	font-size: 18px;
}
</style>

