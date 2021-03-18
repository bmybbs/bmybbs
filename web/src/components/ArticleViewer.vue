<template>
	<div class="card border-bmy-blue1 mb-4">
		<div class="card-header bg-gradient">
			<div class="d-flex">
				<span class="fw-bold text-bmy-dark8">{{ title }}</span>
				<BadgeArticleFlags :_accessed="_mark" />
			</div>

			<div class="text-bmy-dark6 fs-7">
				{{author}} 发表于 <TooltipTimestamp :_unix_timestamp="_aid" />
			</div>
		</div>
		<div class="card-body">
			<div class="article" v-html="content" @click="toggleAnsi" ref="article"></div>
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

export default {
	data() {
		return {
			ansi_list: [],
			show_ansi: true,
			content: "",
			author: "",
			title: "",
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
			this.author = response.author;
			this.title = response.title;
			setTimeout(() => {
				this.ansi_list = [].slice.call(this.$refs.article.querySelectorAll("span.bmybbs-ansi"));
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
		toggleAnsi() {
			if (this.show_ansi) {
				this.show_ansi = false;
				this.ansi_list.forEach((x) => {
					x.classList.remove("bmybbs-ansi");
				});
			} else {
				this.show_ansi = true;
				this.ansi_list.forEach((x) => {
					x.classList.add("bmybbs-ansi");
				});
			}
		},
	},
	components: {
		BadgeArticleFlags,
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

.article:deep(blockquote) {
	padding: 0.5rem 1rem;
	border-left: 5px solid var(--bs-bmy-grey2);
	color: var(--bs-gray);
	background: var(--bs-bmy-grey1);
}

.article:deep(blockquote p) {
	margin-bottom: 0.1rem;
	margin-block-end: 0.1rem;
}
</style>

