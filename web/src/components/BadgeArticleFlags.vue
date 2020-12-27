<template>
	<span>
		<span class="badge rounded-pill bg-danger" v-show="is_mark">mark</span>
		<span class="badge rounded-pill bg-success" v-show="is_digest">文摘</span>
		<span class="badge rounded-pill bg-warning text-dark" v-show="no_reply">不可回复</span>
		<span class="badge rounded-pill bg-secondary" v-show="is_water">水文</span>
	</span>
</template>

<script>
import { ARTICLE_FLAGS } from "@/lib/BMYConstants.js"

export default {
	data() {
		return {
		};
	},
	props: {
		_accessed: Number,
	},
	computed: {
		is_digest() {
			return this._accessed & ARTICLE_FLAGS.FH_DIGEST;
		},
		is_mark() {
			return this._accessed & ARTICLE_FLAGS.FH_MARKED;
		},
		no_reply() {
			return this._accessed & ARTICLE_FLAGS.FH_NOREPLY;
		},
		is_water() {
			return this._accessed & ARTICLE_FLAGS.FH_ISWATER;
		},
	},
}
</script>

<style scoped>
.badge { margin-left: 5px; }
</style>

