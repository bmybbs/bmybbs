<template>
	<div class="card">
		<div class="card-header">
			{{author}} 发表于 <TooltipTimestamp :_unix_timestamp="_aid" />
			<BadgeArticleFlags :_accessed="_mark" />
		</div>
		<div class="card-body">
			<div class="article" v-html="content" @click="toggleAha" ref="article"></div>
		</div>
		<div class="card-footer">
			<TabbedEditor />
		</div>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import TooltipTimestamp from "@/components/TooltipTimestamp.vue"
import BadgeArticleFlags from "@/components/BadgeArticleFlags.vue"
import TabbedEditor from "@/components/TabbedEditor.vue"
import bmyParser from "@bmybbs/bmybbs-content-parser"
import Prism from "prismjs"

export default {
	data() {
		return {
			aha_list: [],
			show_ansi: true,
			content: "",
			author: "",
		}
	},
	props: {
		_boardname_en: String,
		_aid: Number,
		_mark: Number,
	},
	mounted() {
		BMYClient.get_article_content(this._boardname_en, this._aid).then(response => {
			this.content = bmyParser({
				text: response.content,
				attaches: response.attach
			});
			this.aha_list = [].slice.call(this.$refs.article.querySelectorAll("span.aha"));
			this.author = response.author;
			setTimeout(() => {
				Prism.highlightAll();
			}, 1500);
		});
	},
	methods: {
		toggleAha() {
			if (this.show_ansi) {
				this.show_ansi = false;
				this.aha_list.forEach((x) => {
					x.classList.remove("aha");
				});
			} else {
				this.show_ansi = true;
				this.aha_list.forEach((x) => {
					x.classList.add("aha");
				});
			}
		},
	},
	components: {
		BadgeArticleFlags,
		TabbedEditor,
		TooltipTimestamp,
	},
}
</script>

<style scoped>
</style>

