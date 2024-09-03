<script lang="ts">
	import { AppBar, AppShell, LightSwitch } from "@skeletonlabs/skeleton";
	import "../app.postcss";
</script>

<!-- App Shell -->
<AppShell>
	<svelte:fragment slot="header">
		<!-- App Bar -->
		<AppBar>
			<svelte:fragment slot="lead">
				<strong class="text-2xl capitalize">Risk Dice</strong>
			</svelte:fragment>
			<div class="flex justify-center gap-4">
				<a
					class="btn btn-sm bg-amber-400 text-surface-900 font-bold"
					href="https://buymeacoffee.com/masflam"
					target="_blank"
					rel="noreferrer"
				>
					Donate
				</a>
				<a
					class="btn btn-sm bg-surface-200-700-token"
					href="https://links.masflam.com"
					target="_blank"
					rel="noreferrer"
				>
					Author
				</a>
				<a
					class="btn btn-sm bg-surface-200-700-token"
					href="https://github.com/MasFlam/risk-dice-tools"
					target="_blank"
					rel="noreferrer"
				>
					Source Code
				</a>
			</div>
			<svelte:fragment slot="trail">
				<LightSwitch />
			</svelte:fragment>
		</AppBar>
	</svelte:fragment>
	<!-- Page Route Content -->
	<slot />
</AppShell>
