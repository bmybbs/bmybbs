<template>
	<section class="card bmy-dashboard-card">
		<div class="card-header">
			{{ _name }}
		</div>
		<ul class="list-group list-group-flush">
			<DashboardArticleListItem
				v-for="article in _articles"
				:key="article.aid"
				:_board="article.board"
				:_title="article.title"
				:_author="article.author"
				:_comments="article.comments"
				:_aid="article.aid"
			/>
		</ul>
	</section>
</template>

<script>
import DashboardArticleListItem from "@/components/DashboardArticleListItem.vue"

export default {
	data() {
		return { };
	},
	components: {
		DashboardArticleListItem,
	},
	props: {
		_name: String,
		_articles: Array,
	},
}
</script>

