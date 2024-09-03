export function formatPercentage(value: number, precision: number = 3): string {
	return (value * 100).toPrecision(precision) + "%";
}
