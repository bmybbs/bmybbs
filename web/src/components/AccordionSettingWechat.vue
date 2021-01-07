<template>
	<div class="accordion-item">
		<h2 class="accordion-header bg-white text-dark">
			<button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#setting-wechat">微信关联</button>
		</h2>
		<div id="setting-wechat" class="accordion-collapse collapse bg-white">

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
								<button type="button" class="btn btn-danger" data-bs-dismiss="modal" @click="remove">确定移除</button>
							</div>
						</div>
					</div>
				</div>
			</div>

			<div v-else>
				<div class="d-flex flex-column flex-sm-row">
					<div>
						<canvas ref="canvas"></canvas>
					</div>
					<div>
						<div class="text">
							<div>请在微信小程序关联 BMY 账户页扫描<span class="d-none d-sm-inline">左侧</span><span class="d-inline d-sm-none">上方</span>二维码，或者手动输入以下代码获取校验码</div>
							<div class="font-monospace">{{msg}}</div>
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

							<div class="col-auto">
								<button class="btn btn-secondary" @click="load">刷新</button>
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
		this.load();
	},
	methods: {
		load() {
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
				switch (response.errcode) {
				case BMY_EC.API_RT_SUCCESSFUL:
					this.hasopenid = false;
					this.load();
					break;
				case BMY_EC.API_RT_NOTLOGGEDIN:
					this.$toast.error("请重新登录", {
						position: "top"
					});
					break;
				case BMY_EC.API_RT_NOOPENID:
					this.$toast.error("您没有关联微信", {
						position: "top"
					});
					this.hasopenid = false;
					this.load();
					break;
				default:
					this.$toast.error("未知错误", {
						position: "top"
					});
					break;
				}
			});
		},
	},
}
</script>

<style scoped>
.text, .code {
	padding-top: 10px;
}
</style>

