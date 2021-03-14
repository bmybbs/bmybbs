<template>
	<li class="list-group-item border-bmy-blue">
		<div>
			<router-link :to="{ name: 'thread', params: { boardname: _boardname_en, tid: _aid }}" class="fs-5 text-bmy-dark8 text-decoration-none">{{ _title }}</router-link>
			<BadgeArticleFlags :_accessed="_accessed" />
		</div>
		<div class="meta d-flex justify-content-between justify-content-sm-start">
			<span>
				<span class="author"><span class="icon"><fa icon="at" /></span><PopoverUserInfo :_userid="_author" /></span>
				<span class="post">发表于</span>
				<span class="time"><TooltipTimestamp :_unix_timestamp="_aid" /></span>
			</span>

			<span class="dot d-none d-sm-inline-block">•</span>

			<span class="comments"><span class="icon"><fa icon="comments" /></span>{{ _comments }}篇讨论</span>
		</div>
	</li>
</template>

<script>
import { defineAsyncComponent } from "vue"

const BadgeArticleFlags = defineAsyncComponent(() => import("./BadgeArticleFlags.vue"));
const TooltipTimestamp = defineAsyncComponent(() => import("./TooltipTimestamp.vue"));
const PopoverUserInfo = defineAsyncComponent(() => import("./PopoverUserInfo.vue"));

export default {
	data() {
		return { };
	},
	props: {
		_boardname_en: String,
		_title: String,
		_author: String,
		_comments: Number,
		_aid: Number,
		_accessed: Number,
	},
	components: {
		BadgeArticleFlags,
		PopoverUserInfo,
		TooltipTimestamp,
	},
}
</script>

<style scoped>
* {
	font-size: 12px;
}

h3 {
	color: #222222;
	font-size: 16px;
	font-weight: 500;
}

.icon {
	margin-right: 2px;
}

.meta {
	color: #787c7e;
	font-weight: 400;
}

.dot {
	margin: 0 4px;
	font-size: 8px;
}

.post {
	margin: 0 4px;
}
</style>

