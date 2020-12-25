<template>
	<div class="row">
		<div class="col-md-9">
			<router-view />
		</div>
		<div class="col-md-3" v-if="info">
			<CardBoardInfo
				v-bind:_boardname_en="$route.params.boardname"
				v-bind:_boardname_zh="info.zh_name"
				v-bind:_secstr="info.secstr"
				v-bind:_article_num="info.article_num"
				v-bind:_thread_num="info.thread_num"
				v-bind:_inboard_num="info.inboard_num"
				v-bind:_today_new="info.today_new"
			/>
		</div>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import CardBoardInfo from "@/components/CardBoardInfo.vue"

export default {
	data() {
		return {
			info: null,
		};
	},
	created() {
		this.$watch(() => this.$route.params, (toParams) => {
			this.get_info(toParams.boardname);
		});
	},
	mounted() {
		this.get_info(this.$route.params.boardname);
	},
	methods: {
		get_info(boardname) {
			BMYClient.get_board_info(boardname).then(response => {
				this.info = response;
			});
		},
	},
	components: {
		CardBoardInfo,
	},
}
</script>

<style scoped>
</style>

