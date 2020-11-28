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
							v-bind:key="article.aid"
							v-bind:_board="article.board"
							v-bind:_title="article.title"
							v-bind:_author="article.author"
							v-bind:_comments="article.comments"
							v-bind:_aid="article.aid"
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

export default {
	data() {
		return {
			articles: [
				{ board: "XJTUnews", title: "Lorem ipsum dolor sit amet", author: "foo", comments: 42, aid: 1606538804, },
				{ board: "doctor", title: "Lorem ipsum dolor sit amet", author: "foo", comments: 20, aid: 1606538704, },
				{ board: "XJTUsis", title: "Lorem ipsum dolor sit amet", author: "bar", comments: 42, aid: 1606538604, },
				{ board: "XJTUei", title: "Lorem ipsum dolor sit amet", author: "baz", comments: 42, aid: 1606538504, },
				{ board: "XJTUsce", title: "Lorem ipsum dolor sit amet", author: "SYSOP", comments: 7, aid: 1606538404, },
				{ board: "XJTUnews", title: "Lorem ipsum dolor sit amet", author: "SYSOP", comments: 9, aid: 1606538304, },
				{ board: "newlife", title: "Lorem ipsum dolor sit amet", author: "foo", comments: 12, aid: 1606538204, },
				{ board: "newlife", title: "Lorem ipsum dolor sit amet", author: "foo", comments: 23, aid: 1606538104, },
			],
		}
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

