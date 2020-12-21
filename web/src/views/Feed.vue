<template>
	<div class="container-fluid">
		<div class="row">
			<div class="col-xs-6 col-md-6">
				<h2>订阅</h2>
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
			<div class="col-md-12">
				<div class="card">
					<ul class="list-group list-group-flush">
						<DashboardArticleListItem
							v-for="article in articles"
							v-bind:key="[article.boardname_en, '/', article.aid].join('')"
							v-bind:_board="article.boardname_zh"
							v-bind:_title="article.title"
							v-bind:_author="article.author"
							v-bind:_comments="article.count"
							v-bind:_aid="article.tid"
						/>
					</ul>
					<div class="card-footer text-center">
						加载更多
					</div>
				</div>
			</div>
		</div>
	</div>
</template>

<script>
import DashboardArticleListItem from "@/components/DashboardArticleListItem.vue"
import { BMYClient } from "@/lib/BMYClient.js"

export default {
	data() {
		return {
			time: Math.floor(new Date().getTime() / 1000),
			articles: [ ],
		}
	},
	mounted() {
		BMYClient.get_feed(this.time).then(response => {
			if (Array.isArray(response.articles)) {
				this.articles = response.articles;
			}

			// TODO
		});
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

