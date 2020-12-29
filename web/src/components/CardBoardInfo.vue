<template>
	<div>
		<div class="title">
			<span class="title-main">{{ _boardname_zh }}</span>
			<span class="title-sec">{{ _secstr }}/{{ _boardname_en }}</span>
		</div>

		<div class="numbers block">
			<div class="numbers-item">
				<div class="numbers-item-number">{{ kFormatter(_thread_num) }}</div>
				<div class="numbers-item-text">主题</div>
			</div>
			<div class="numbers-item">
				<div class="numbers-item-number">{{ kFormatter(_article_num) }}</div>
				<div class="numbers-item-text">帖子</div>
			</div>
			<div class="numbers-item">
				<div class="numbers-item-number">{{ kFormatter(_today_new) }}</div>
				<div class="numbers-item-text">新增</div>
			</div>
			<div class="numbers-item">
				<div class="numbers-item-number">{{ kFormatter(_inboard_num) }}</div>
				<div class="numbers-item-text">在线</div>
			</div>
		</div>

		<div class="intro block">
			<h3>简介</h3>
			<p>{{ _intro }}</p>
		</div>

		<div class="keyword block">
			<h3>关键字</h3>
		</div>

		<div class="hot block">
			<h3>热门话题</h3>
		</div>
	</div>
</template>

<script>
export default {
	data() {
		return {};
	},
	props: {
		_boardname_en: String,
		_boardname_zh: String,
		_secstr: String,
		_article_num: Number,
		_thread_num: Number,
		_today_new: Number,
		_inboard_num: Number,
	},
	methods: {
		kFormatter(num) {
			if (num > 999)
				return (num/1000).toFixed(1) + "k";
			else
				return num;
		},
	},
}
</script>

<style scoped>
.title-main {
	font-size: 20px;
	font-weight: 600;
}
.numbers {
	display: flex;
	justify-content: space-between;
}

.numbers-item {
	display: flex;
	flex-direction: column;
	align-items: center;
	margin-left: 10px;
}

.numbers-item:first-child {
	margin-left: 0px;
}

.numbers-item-number {
	font-size: 18px;
	font-weight: 600;
}

.numbers-item-text {
	font-size: 12px;
}

h3 {
	font-size: 14px;
	font-weight: 600;
	color: #1c1c1c;
}

.block {
	margin-top: 5px;
}
</style>

