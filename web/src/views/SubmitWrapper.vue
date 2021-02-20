<template>
	<div class="row">
		<div class="col-md-9">
			<router-view />
		</div>

		<div class="col-md-3">
			<div class="card">
				<div class="card-body">
					<h5 class="card-title">在 BMYBBS 创建内容</h5>
					<p>请留意相关版面的讨论规范。</p>
					<p>发文时应慎重考虑文章内容是否适合公开场合发表。</p>
					<p>谢谢您的合作。</p>
				</div>
			</div>
		</div>
	</div>
</template>

<script>
export default {
	date() {
		return {};
	},
}
</script>

