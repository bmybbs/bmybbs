<template>
	<div class="article" v-html="content"></div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"

export default {
	data() {
		return {
			content: "",
		}
	},
	props: {
		_boardname_en: String,
		_aid: Number,
	},
	mounted() {
		BMYClient.get_article_content(this._boardname_en, this._aid).then(response => {
			this.content = response.content;
		});
	}
}
</script>

<style scoped>
.article {
	border: 1px solid #bbb;
	border-radius: 5px;
	padding: 20px;
	margin: 20px 0;
	font-size: 14px;
}
</style>

