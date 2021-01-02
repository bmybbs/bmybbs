<template>
	<div class="accordion-item">
		<h2 class="accordion-header">
			<button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#setting-wechat">微信关联</button>
		</h2>
		<div id="setting-wechat" class="accordion-collapse collapse">

			<div v-if="hasopenid">
				您已关联微信账号
				<button class="btn btn-danger" data-bs-toggle="modal" data-bs-target="#settingWechatModal">解除关联</button>
				<!-- modal -->
				<div class="modal fade" id="settingWechatModal" tabindex="-1">
					<div class="modal-dialog">
						<div class="modal-content">
							<div class="modal-header">
								<h5 class="modal-title">确认移除当前微信关联吗？</h5>
								<button type="button" class="btn-close" data-bs-dismiss="modal"></button>
							</div>
							<div class="modal-footer">
								<button type="button" class="btn btn-secondary" data-bs-dismiss="modal">取消</button>
								<button type="button" class="btn btn-danger" @click="remove">确定移除</button>
							</div>
						</div>
					</div>
				</div>
			</div>

			<div v-else>
				<div class="setting-container">
					<div>
						<canvas ref="canvas"></canvas>
					</div>
					<div>
						<div class="text">
							<div>请在微信小程序关联 BMY 账户页扫描左侧二维码，或者手动输入以下代码获取校验码</div>
							<div class="key">{{msg}}</div>
						</div>
						<div class="code row">
							<div class="col-auto">
								<label for="wechat2FACode" class="col-form-label">校验码</label>
							</div>

							<div class="col-auto">
								<input type="text" v-model="code" id="wechat2FACode" class="form-control" placeholder="一串 6 位数字">
							</div>

							<div class="col-auto">
								<button class="btn btn-primary" @click="submit">提交</button>
							</div>
						</div>
					</div>
				</div>
			</div>

		</div>
	</div>
</template>

<script>
import QRCode from "qrcode"
import { BMYClient } from "@/lib/BMYClient.js"
import { BMY_EC } from "@/lib/BMYConstants.js"

export default {
	data() {
		return {
			msg: "",
			code: "",
			hasopenid: false,
		};
	},
	mounted() {
		BMYClient.oauth_get_key().then(response => {
			if (response.errcode == BMY_EC.API_RT_SUCCESSFUL) {
				QRCode.toCanvas(this.$refs.canvas, response.key, function(error) {
					if (error)
						console.error(error);
				});
				this.msg = response.key;
			} else if (response.errcode == BMY_EC.API_RT_HASOPENID) {
				this.hasopenid = true;
			}
		});
	},
	methods: {
		submit() {
			BMYClient.oauth_check_code(this.code).then(response => {
				switch (response.errcode) {
				case BMY_EC.API_RT_SUCCESSFUL:
					this.hasopenid = true;
					break;
				}
			});
		},
		remove() {
			BMYClient.oauth_remove_wx().then(response => {
				if (response.errcode == BMY_EC.API_RT_SUCCESSFUL) {
					this.hasopenid = false;
				}
			});
		},
	},
}
</script>

<style scoped>
.setting-container {
	display: flex;
}

.text, .code {
	padding-top: 10px;
}

.key {
	display: block;
	font-family: consolas, "Courier New", monospace;
}
</style>

