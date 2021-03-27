<template>
	<div>
		<nav>
			<ul class="pagination">
				<li class="page-item" :class="{ active: button.active }" v-for="button in navButtons" :key="button.text" @click="doJump(button)">
					<span class="page-link">{{ button.text }}</span>
				</li>
			</ul>
		</nav>
	</div>
</template>

<script>

const STR_FIRST = "«",
	STR_LAST = "»",
	STR_PREV = "‹",
	STR_NEXT = "›";

class PButton {
	constructor(text, enabled = false, active = false) {
		this.text = text;
		this.enabled = enabled;
		this.active = active;
	}
}

const generateButtonList = (current, total) => {
	current = Math.ceil(current);
	if (current < 1 || total < 1) {
		return [
			new PButton(STR_FIRST),
			new PButton(STR_PREV),
			new PButton(STR_NEXT),
			new PButton(STR_LAST),
		];
	}

	if (current > total)
		current = total;

	if (total == 1) {
		return [
			new PButton(STR_FIRST),
			new PButton(STR_PREV),
			new PButton(current, false, true),
			new PButton(STR_NEXT),
			new PButton(STR_LAST),
		];
	} else if (total == 2) {
		const b_1 = (current == 1),
			b_2 = (current == 2);
		return [
			new PButton(STR_FIRST, !b_1),
			new PButton(STR_PREV, !b_1),
			new PButton(1, !b_1, b_1),
			new PButton(2, !b_2, b_2),
			new PButton(STR_NEXT, !b_2),
			new PButton(STR_LAST, !b_2),
		];
	} else {
		if (current == 1) {
			return [
				new PButton(STR_FIRST),
				new PButton(STR_PREV),
				new PButton(1, false, true),
				new PButton(2, true),
				new PButton(3, true),
				new PButton(STR_NEXT),
				new PButton(STR_LAST),
			];
		} else if (current == total) {
			return [
				new PButton(STR_FIRST, true),
				new PButton(STR_PREV, true),
				new PButton(total - 2, true),
				new PButton(total - 1, true),
				new PButton(total, false, true),
				new PButton(STR_NEXT),
				new PButton(STR_LAST),
			];
		} else {
			return [
				new PButton(STR_FIRST, true),
				new PButton(STR_PREV, true),
				new PButton(current - 1, true),
				new PButton(current, false, true),
				new PButton(current + 1, true),
				new PButton(STR_NEXT, true),
				new PButton(STR_LAST, true),
			];
		}
	}
};

export default {
	data() {
		return {
		};
	},
	props: {
		_total: {
			type: Number,
			default: 1,
		},
		_current: {
			type: Number,
			default: 1,
		},
		_callback: {
			type: Function,
			default: (_) => console.log(_)
		}
	},
	computed: {
		navButtons() {
			return generateButtonList(this._current, this._total);
		}
	},
	methods: {
		doJump(button) {
			if (!button.enabled)
				return;

			const current = Math.ceil(+this._current);
			let target;
			switch (button.text) {
			case STR_FIRST: target = 1;            break;
			case STR_LAST:  target = this._total;  break;
			case STR_PREV:  target = current - 1;  break;
			case STR_NEXT:  target = current + 1;  break;
			default:        target = button.text;
			}

			this._callback(target);
		}
	},
}
</script>

