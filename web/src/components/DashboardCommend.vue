<template>
	<section class="card mb-4 bmy-dashboard-card border-bmy-blue1">
		<div class="card-header bg-bmy-blue1 bg-gradient text-white fs-5">
			{{ _name }}
			<span class="float-end text-white font-monospace"><fa :icon="_icon" /></span>
		</div>
		<ul class="list-group list-group-flush">
			<DashboardArticleListItem
				v-for="article in _articles"
				:key="article.aid"
				:_boardname_en="article.board"
				:_boardname_zh="article.board"
				:_title="article.title"
				:_author="article.author"
				:_comments="article.comments"
				:_aid="article.aid"
			/>
		</ul>
	</section>
</template>

<script>
import { defineAsyncComponent } from "vue"
const DashboardArticleListItem = defineAsyncComponent(() => import("./DashboardArticleListItem.vue"));

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
		_icon: String,
	},
}
</script>

