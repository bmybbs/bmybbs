<template>
	<div class="card">
		<div class="card-header">
			{{author}} 发表于 <TooltipTimestamp :_unix_timestamp="_aid" />
			<BadgeArticleFlags :_accessed="_mark" />
		</div>
		<div class="card-body">
			<div class="article" v-html="content" @click="toggleAha"></div>
		</div>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import TooltipTimestamp from "@/components/TooltipTimestamp.vue"
import BadgeArticleFlags from "@/components/BadgeArticleFlags.vue"

export default {
	data() {
		return {
			v_dom: null,
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
			this.content = response.content;
			this.v_dom = document.createElement('div');
			this.v_dom.innerHTML = response.content;
			this.aha_list = [].slice.call(this.v_dom.querySelectorAll("span.aha"));
			this.author = response.author;
		});
	},
	methods: {
		toggleAha() {
			if (this.show_ansi) {
				this.show_ansi = false;
				this.aha_list.map((x) => {
					x.classList.remove("aha");
				});
			} else {
				this.show_ansi = true;
				this.aha_list.map((x) => {
					x.classList.add("aha");
				});
			}
			this.content = this.v_dom.innerHTML;
		},
	},
	components: {
		BadgeArticleFlags,
		TooltipTimestamp,
	},
}
</script>

<style scoped>
</style>

