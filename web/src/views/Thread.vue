<template>
	<div>
		<ArticleViewer v-for="article in articlelist"
			v-bind:key="article.aid"
			v-bind:_boardname_en="$route.params.boardname"
			v-bind:_aid="article.aid"
		/>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import ArticleViewer from "@/components/ArticleViewer.vue"

export default {
	data() {
		return {
			articlelist: [],
		};
	},
	mounted() {
		BMYClient.get_thread_list(this.$route.params.boardname, this.$route.params.tid).then(response => {
			if (response.errcode == 0) {
				this.articlelist = response.articlelist;
			}
		});
	},
	components: {
		ArticleViewer,
	},
}
</script>

<style scoped>
</style>

