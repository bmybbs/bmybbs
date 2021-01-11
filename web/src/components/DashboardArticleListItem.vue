<template>
	<li class="list-group-item">
		<div class="d-flex justify-content-between">
			<router-link :to="{ name: 'thread', params: { boardname: _boardname_en, tid: _aid }}" class="fs-5 fw-bold text-dark text-decoration-none">{{ _title }}</router-link>
			<BadgeArticleFlags :_accessed="_accessed" />
		</div>
		<div class="meta d-flex justify-content-between justify-content-sm-start">
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
import BadgeArticleFlags from "@/components/BadgeArticleFlags.vue"
import TooltipTimestamp from "@/components/TooltipTimestamp.vue"
import PopoverBoardInfo from "@/components/PopoverBoardInfo.vue"
import PopoverUserInfo from "@/components/PopoverUserInfo.vue"

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

.meta >>> * {
	color: #787c7e;
	font-size: .9rem;
	font-weight: 400;
}

.board >>> * {
	color: #1c1c1c;
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

