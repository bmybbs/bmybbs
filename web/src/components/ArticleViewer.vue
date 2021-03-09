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
			<TabbedEditor :_boardname_en="_boardname_en"/>
		</div>
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
import { BMYClient } from "@/lib/BMYClient.js"
import bmyParser from "@bmybbs/bmybbs-content-parser"
import Prism from "prismjs"
import { BMY_FILE_HEADER } from "@/lib/BMYConstants.js"
import "@/plugins/mathjax.js"

const TooltipTimestamp = defineAsyncComponent(() => import("./TooltipTimestamp.vue"));
const BadgeArticleFlags = defineAsyncComponent(() => import("./BadgeArticleFlags.vue"));
const TabbedEditor = defineAsyncComponent(() => import("./TabbedEditor.vue"));

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
				if (this._mark & BMY_FILE_HEADER.FH_MATH) {
					if (window.MathJax && typeof window.MathJax.typeset === "function") {
						window.MathJax.typeset();
					}
				}
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

<style>
mjx-container {
	font-size: 120%
}
</style>

<style scoped>
.article:deep(img) {
	max-width: 100%
}
</style>

