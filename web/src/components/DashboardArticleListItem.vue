<template>
	<li class="list-group-item">
		<span class="board">{{ _board }}</span>
		<span class="title">{{ _title }}</span>
		<span class="author"><span class="icon"><fa icon="at" /></span>{{ _author }}</span>
		<span class="comments"><span class="icon"><fa icon="comments" /></span>{{ _comments }}</span>
		<span class="time"><span class="icon"><fa icon="clock" /></span>{{ readableTime }}</span>
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
		_board: String,
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
li > span:not(.title) {
	color: #777;
}

li > span:not(.time) {
	margin-right: 15px;
}

li > span:not(.title):hover {
	cursor: pointer;
	color: #333;
}

.board {
	display: inline-block;
	width: 150px;
}

.time {
	display: inline-block;
	position: absolute;
	right: 16px;
}

.icon {
	margin-right: 2px;
}
</style>

