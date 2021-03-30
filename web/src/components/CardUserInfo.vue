<template>
	<div v-if="info">
		<div class="title">
			<span class="userid">{{ info.userid }}</span>
			<span>{{ info.exp_level }}</span>
			<span>{{ info.job }}</span>
		</div>
		<div class="figure">
			<div class="item">
				<div class="number">{{ info.login_counts }}</div>
				<div class="text">次上线</div>
			</div>

			<div class="item">
				<div class="number">{{ info.post_counts }}</div>
				<div class="text">篇文章</div>
			</div>
		</div>
	</div>
	<div v-else></div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
export default {
	data() {
		return {
			info: null,
		};
	},
	mounted() {
		BMYClient.get_user_info(this._userid).then(response => {
			this.info = response;
		});
	},
	props: {
		_userid: String,
	}
}
</script>

<style scoped>
.userid {
	font-weight: 700;
}

.figure {
	display: flex;
	justify-content: space-between;
}

.figure .item {
	display: flex;
}

.figure .item:not(:first-child) {
	margin-left: 10px;
}

.figure .number {
	font-weight: 700;
}
</style>

