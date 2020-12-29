<template>
	<span ref="span" data-bs-toggle="tooltip" data-bs-placement="top" :title="longVersionTime">{{ readableTime }}</span>
</template>

<script>
import { Tooltip } from "bootstrap"
import dayjs from "dayjs"
import relativeTime from "dayjs/plugin/relativeTime"
import localizedFormat from "dayjs/plugin/localizedFormat"
import "dayjs/locale/zh-cn"

dayjs.locale("zh-cn");
dayjs.extend(relativeTime);
dayjs.extend(localizedFormat);

export default {
	data() {
		return {};
	},
	props: {
		_unix_timestamp: Number,
	},
	mounted() {
		new Tooltip(this.$refs.span);
	},
	computed: {
		readableTime: function() {
			return dayjs(this._unix_timestamp * 1000).fromNow();
		},
		longVersionTime: function() {
			return dayjs(this._unix_timestamp * 1000).format("LLLL");
		},
	},
}
</script>

