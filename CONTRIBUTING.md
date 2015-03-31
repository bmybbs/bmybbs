# 参与 BMYBBS 代码建设

## 在开始之前

BMYBBS 的源代码遵循 [GNU General Public License version 2](https://www.gnu.org/licenses/gpl-2.0.html) 许可协议，因此任何的 pull request、缺陷汇报都将被视为在此协议下进行。

## 缺陷反馈

1. 更新至 master 的代码，缺陷可能已被修复。

2. 请提供重现缺陷所需的详细步骤。

## 新功能申请

请勿在 GitHub 的 Issue 中提交新功能的申请。Issue 中您可能会看到一些新功能的任务，但那些是经过讨论后确定下来要完成的开发任务。请转至 [BMYBBS/BMY_DEV](http://www.bmybbs.com/BMY/home?B=BMY_DEV) 版面提交新功能申请。您需要详细的描述功能，以及提交的原因，并参与后续的讨论。

## 参与代码编写

1. 克隆代码到您的计算机，并创建新的分支：<br />

        git clone git@github.com:bmybbs/bmybbs.git
        cd bmybbs
        git checkout -b branch_name

  `branch_name` 请尽量取得有意义一些，例如 `fix-issue-30`、`fix-bug-when-doing-sth` 或者 `add-feature-xyz`。

  请保持代码的整洁，一个分支仅解决一项任务。如果有多项任务计划完成，请创建独立的分支。

2. 编码。请遵循这样的编码格式约定：
    * 使用 tab 缩进，而非空格；
    * 不要遗留多余的空白，空行中不要出现 tab 或者空格；
    * 在运算符前后，以及逗号、分号、冒号之后，以及 `{` 前后和 `}` 之前使用空格；
    * `(`，`[` 之后以及 `]`，`)` 之前不要使用空格；
    * 函数的 `{` 放到新的一行中，其他情况下与之前的代码放在同一行，如下所示；
    * [bmybbs](https://github.com/bmybbs/bmybbs) 的代码使用的是 GBK 编码，[api](https://github.com/bmybbs/api) 和 [web](https://github.com/bmybbs/web) 使用 UTF-8 编码；

    ```C
    int my_compare (int a, int b)
    {
    	if (a > 0) {
    		return b;
    	} else {
    		return a;
    	}
    }
    ```

3. 提交。每次 `git commit` 的时候请使用简短的概括，请勿留空。

4. Fork。在 clone 项目（例如本例中 bmybbs）的 GitHub 页面中，点击 **Fork** 按钮，这样 GitHub 上会产生您自己的项目。然后，在本地代码目录下执行：

  ```
  git remote add mine git@github.com:<your user name>/bmybbs.git
  ```

  Fork 仅需执行一次。

5. 上传代码并创建 pull request。

  从上游同步代码，并合并本地的变更，再上传到刚刚 Fork 出来的项目中。

  ```
  git fetch origin
  git checkout branch_name
  git rebase origin/master
  git push -f mine branch_name
  ```

  请将 `branch_name` 替换为您实际的分支名称。

  然后，请访问您 Fork 出来的项目页面，例如 <u>github.com/**your-user-name**/bmybbs</u>，在页面上点击 **Pull Request**并创建新的 PR。选择 base 的分支名（一般为 master），以及提交变更的分支名（例如本例的 `branch_name`）。您可以在这个页面上检查所有的变更。请附加说明，并起一个具体的、有意义的标题，然后完成 PR 创建。通常我们会立即收到邮件提醒，并做 code review，请关注后续进程。

感谢您的贡献。
