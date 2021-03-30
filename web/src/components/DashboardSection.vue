<template>
	<section class="card mb-4 border-bmy-blue1">
		<div class="card-header bg-bmy-blue1 bg-gradient fs-5">
			<router-link :to="{ name: 'section', params: { secid: _secstr }}" class="text-white text-decoration-none">{{ _name }}</router-link>
			<span class="float-end text-white font-monospace">{{_secstr}}</span>
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
				:_accessed="article.mark"
			/>
		</ul>
	</section>
</template>

<script>
import { defineAsyncComponent } from "vue"
import { BMYClient } from "@/lib/BMYClient.js"
const DashboardArticleListItem = defineAsyncComponent(() => import("./DashboardArticleListItem.vue"));

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

