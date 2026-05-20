from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

RESULTS = Path("results")
CSV_PATH = RESULTS / "errors.csv"


def clip_values(values: np.ndarray) -> np.ndarray:
    return np.maximum(values, 1.0e-30)


def make_panel(data: np.ndarray, y_forward: str, y_backward: str, ylabel: str, outfile: Path) -> None:
    xs = np.unique(data["x"])

    fig, axes = plt.subplots(2, 2, figsize=(11, 8), sharex=True)
    axes = axes.ravel()

    for ax, x in zip(axes, xs):
        subset = data[data["x"] == x]
        ell = subset["l"]
        ax.semilogy(ell, clip_values(subset[y_forward]), marker="o", markersize=2.5, linewidth=1.2, label="forward")
        ax.semilogy(ell, clip_values(subset[y_backward]), marker="s", markersize=2.5, linewidth=1.2, label="backward")
        ax.set_title(f"x = {x:g}")
        ax.set_xlabel(r"$\ell$")
        ax.set_ylabel(ylabel)
        ax.grid(True, which="both", alpha=0.3)
        ax.legend(frameon=False)

    fig.tight_layout()
    fig.savefig(outfile, dpi=220, bbox_inches="tight")
    plt.close(fig)


def main() -> None:
    data = np.genfromtxt(CSV_PATH, delimiter=",", names=True)
    make_panel(data, "rel_err_forward", "rel_err_backward", "relative error", RESULTS / "relative_error_panel.png")
    make_panel(data, "abs_err_forward", "abs_err_backward", "absolute error", RESULTS / "absolute_error_panel.png")


if __name__ == "__main__":
    main()
