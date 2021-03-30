<template>
	<span class="d-inline-block bg-bmy-grey1 px-4 py-1 me-3 mb-3 rounded-pill">{{ _text }}</span>
</template>

<script>
export default {
	data() {
		return {};
	},
	props: {
		_text: String,
	},
}
</script>

<style scoped>
span {
	cursor: pointer;
}
span:hover {
	background-color: var(--bs-bmy-grey2) !important;
}
</style>

