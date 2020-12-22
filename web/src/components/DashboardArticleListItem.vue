<template>
	<li class="list-group-item">
		<h3>{{ _title }}</h3>
		<div class="meta">
			<span class="board">{{ _boardname_zh }}</span>

			<span class="dot">•</span>

			<span class="author"><span class="icon"><fa icon="at" /></span>{{ _author }}</span>
			<span class="post">发表于</span>
			<span class="time">{{ readableTime }}</span>

			<span class="dot">•</span>

			<span class="comments"><span class="icon"><fa icon="comments" /></span>{{ _comments }}篇讨论</span>
		</div>
	</li>
</template>

<script>
import dayjs from "dayjs"
import relativeTime from "dayjs/plugin/relativeTime"
import "dayjs/locale/zh-cn"

dayjs.locale("zh-cn");
dayjs.extend(relativeTime);

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
	},
	computed: {
		readableTime: function() {
			return dayjs(this._aid * 1000).fromNow();
		}
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

.board {
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

