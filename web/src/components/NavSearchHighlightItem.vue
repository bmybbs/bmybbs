<template>
	<span v-html="html"></span>
</template>

<script>
export default {
	data() {
		return {};
	},
	props: {
		_text: String,
		_key: String,
	},
	computed: {
		html: function() {
			let reg = new RegExp(this._key, "gi");
			return this._text.replaceAll(reg, `<span>$&</span>`);
		},
	},
}
</script>

<style scoped>
span >>> span {
	font-weight: 900;
	color: yellow;
}
</style>

