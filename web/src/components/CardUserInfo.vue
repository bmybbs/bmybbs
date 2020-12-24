<template>
	<div>
		<div class="title">
			<span class="userid">{{ _userid }}</span>
			<span>{{ _exp_level }}</span>
			<span>{{ _job }}</span>
		</div>
		<div class="figure">
			<div class="item">
				<div class="number">{{ _login_counts }}</div>
				<div class="text">次上线</div>
			</div>

			<div class="item">
				<div class="number">{{ _post_counts }}</div>
				<div class="text">篇文章</div>
			</div>
		</div>
	</div>
</template>

<script>
export default {
	data() {
		return {};
	},
	props: {
		_userid: String,
		_exp_level: String,
		_perf_level: String,
		_job: String,
		_login_counts: Number,
		_post_counts: Number,
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

