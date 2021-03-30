<template>
	<div v-if="info">
		<div class="d-flex mb-1">
			<span class="fw-bold me-1">{{ info.userid }}</span>
			<span class="badge rounded-pill bg-secondary">{{ info.job }}</span>
		</div>

		<div class="mb-1 text-secondary">{{ info.nickname }}</div>

		<div class="d-flex justify-content-between mb-1">
			<div class="item d-flex justify-content-between me-1">
				<div>登录</div>
				<div class="badge rounded-pill bg-secondary">{{ info.login_counts }}</div>
			</div>

			<div class="item d-flex justify-content-between">
				<div>发文</div>
				<div class="badge rounded-pill bg-secondary">{{ info.post_counts }}</div>
			</div>
		</div>
		<div class="d-flex justify-content-between">
			<div class="item d-flex justify-content-between me-1">
				<div>经验</div>
				<div class="badge rounded-pill bg-secondary">{{ info.exp_level }}</div>
			</div>

			<div class="item d-flex justify-content-between">
				<div>表现</div>
				<div class="badge rounded-pill bg-secondary">{{ info.perf_level }}</div>
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
.item { width: 10rem; }
</style>

