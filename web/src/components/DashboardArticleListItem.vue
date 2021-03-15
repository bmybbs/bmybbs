<template>
	<li class="list-group-item border-bmy-blue">
		<div class="d-flex">
			<router-link :to="{ name: 'thread', params: { boardname: _boardname_en, tid: _aid }}" class="fs-5 text-bmy-dark8 text-decoration-none">{{ _title }}</router-link>
			<BadgeArticleFlags :_accessed="_accessed" />
		</div>
		<div class="meta d-flex justify-content-between justify-content-sm-start text-bmy-dark6 fs-7">
			<span class="board"><span class="icon"><fa icon="hashtag" /></span><PopoverBoardInfo :_boardname_zh="_boardname_zh" :_boardname_en="_boardname_en" /></span>

			<span class="dot align-self-center d-none d-sm-inline-block">•</span>

			<span class="author"><span class="icon"><fa icon="at" /></span><PopoverUserInfo :_userid="_author" /></span>

			<span>
				<span class="post d-none d-sm-inline-block">发表于</span>
				<span class="time"><TooltipTimestamp :_unix_timestamp="_aid" /></span>
			</span>

			<span class="dot align-self-center d-none d-sm-inline-block">•</span>

			<span class="comments"><span class="icon"><fa icon="comments" /></span>{{ _comments }}<span class="d-none d-sm-inline-block">篇讨论</span></span>
		</div>
	</li>
</template>

<script>
import { defineAsyncComponent } from "vue"

const BadgeArticleFlags = defineAsyncComponent(() => import("./BadgeArticleFlags.vue"));
const TooltipTimestamp = defineAsyncComponent(() => import("./TooltipTimestamp.vue"));
const PopoverBoardInfo = defineAsyncComponent(() => import("./PopoverBoardInfo.vue"));
const PopoverUserInfo = defineAsyncComponent(() => import("./PopoverUserInfo.vue"));

export default {
	data() {
		return { };
	},
	props: {
		_boardname_zh: String,
		_boardname_en: String,
		_title: String,
		_author: String,
		_comments: Number,
		_aid: Number,
		_accessed: Number,
	},
	components: {
		BadgeArticleFlags,
		PopoverBoardInfo,
		PopoverUserInfo,
		TooltipTimestamp,
	},
}
</script>

<style scoped>
.icon {
	margin-right: 2px;
}

.meta:deep(*) {
	font-weight: 400;
}

.board:deep(*) {
	color: var(--bs-bmy-dark6);
	font-weight: 700;
}

.dot {
	margin: 0 4px;
	font-size: 8px;
}

.post {
	margin: 0 4px;
}
</style>

