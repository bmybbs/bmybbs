<template>
	<section class="card mb-4 shadow-lg bmy-dashboard-card">
		<div class="card-header bg-secondary bg-gradient fs-5">
			<router-link :to="{ name: 'section', params: { secid: _secstr }}" class="text-white text-decoration-none">{{ _name }}</router-link>
		</div>
		<ul class="list-group list-group-flush">
			<DashboardArticleListItem
				v-for="article in articlelist"
				:key="[article.board, '/', article.aid].join('')"
				:_boardname_en="article.board"
				:_boardname_zh="article.board"
				:_title="article.title"
				:_author="article.author"
				:_comments="article.th_num"
				:_aid="article.aid"
			/>
		</ul>
	</section>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import DashboardArticleListItem from "@/components/DashboardArticleListItem.vue"

export default {
	data() {
		return {
			articlelist: [],
		};
	},
	mounted() {
		BMYClient.get_sectop(this._secstr).then(response => {
			if (response.errcode == 0) {
				this.articlelist = response.articlelist;
			}
		});
	},
	props: {
		_name: String,
		_secstr: String,
	},
	components: {
		DashboardArticleListItem,
	},
}
</script>

