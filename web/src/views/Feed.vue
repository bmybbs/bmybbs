<template>
	<div class="container-fluid" v-if="loggedin">
		<div class="row">
			<div class="col-xs-6 col-md-6">
				<h2 v-if="feedmode">订阅</h2>
				<h2 v-else>{{$route.params.secid}}区</h2>
			</div>
			<div class="col-xs-6 col-md-6 right-col d-flex align-items-center justify-content-end">
				<span class="icon mr-3">
					<fa icon="align-justify" />
				</span>
				<span class="icon">
					<fa icon="redo" />
				</span>
			</div>
		</div>
		<div class="row">
			<div class="col-md-9">
				<div class="card">
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
						/>
					</ul>
					<div class="card-footer text-center" @click="load_more">
						加载更多
					</div>
				</div>
			</div>
			<div class="col-md-3">
				<div class="card">
					<div v-if="feedmode" class="card-header">
						收藏夹列表
					</div>
					<div v-else class="card-header">
						{{$route.params.secid}}区版面列表
					</div>
					<ul class="list-group list-group-flush">
						<li class="list-group-item"
							v-for="board in favboards"
							v-bind:key="board.name">{{ board.name }}</li>
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
import DashboardArticleListItem from "@/components/DashboardArticleListItem.vue"
import { BMYClient } from "@/lib/BMYClient.js"
import { BOARD_SORT_MODE, BMY_EC } from "@/lib/BMYConstants.js"

export default {
	data() {
		return {
			feedmode: this.$route.name == "feed",
			time: Math.floor(new Date().getTime() / 1000),
			articles: [ ],
			favboards: [ ],
			loggedin: true,
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
		load_more() {
			if (this.feedmode) {
				this.load_feed_more();
			} else {
				this.load_section_more();
			}
		},
		load_feed_more() {
			BMYClient.get_feed(this.time).then(response => {
				switch (response.errcode) {
				case BMY_EC.API_RT_SUCCESSFUL:
					if (Array.isArray(response.articles)) {
						this.articles = this.articles.concat(response.articles);
						let l = response.articles.length;
						this.time = response.articles[l - 1].tid - 1;
					}
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
			BMYClient.get_article_list_by_section(this.$route.params.secid).then(response => {
				if (Array.isArray(response.articles)) {
					this.articles = response.articles;
				}
			});
		},
	},
	components: {
		DashboardArticleListItem,
	}
}
</script>

<style scoped>
span.icon {
	font-size: 18px;
}
</style>

