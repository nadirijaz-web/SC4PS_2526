from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


OUTPUT = Path("output")
FIGURES = Path("figures")
FIGURES.mkdir(exist_ok=True)

plt.style.use("seaborn-v0_8-whitegrid")
plt.rcParams["figure.figsize"] = (8, 4.5)
plt.rcParams["axes.facecolor"] = "white"
plt.rcParams["figure.facecolor"] = "white"
plt.rcParams["axes.edgecolor"] = "#222222"
plt.rcParams["axes.linewidth"] = 1.2
plt.rcParams["axes.spines.top"] = True
plt.rcParams["axes.spines.right"] = True
plt.rcParams["axes.titleweight"] = "bold"
plt.rcParams["axes.labelsize"] = 11
plt.rcParams["axes.titlesize"] = 13
plt.rcParams["legend.frameon"] = True
plt.rcParams["legend.framealpha"] = 1.0
plt.rcParams["legend.edgecolor"] = "#333333"
plt.rcParams["grid.alpha"] = 0.0


BLUE = "#2F6B9A"
RED = "#C84B31"
GREEN = "#2E7D5B"
GOLD = "#B8872D"
DARK = "#222222"


def polish_axes(ax):
    ax.grid(False)
    for spine in ax.spines.values():
        spine.set_visible(True)
        spine.set_color(DARK)
        spine.set_linewidth(1.2)
    ax.tick_params(direction="out", length=4, width=1.0, colors=DARK)
    ax.title.set_color(DARK)
    ax.xaxis.label.set_color(DARK)
    ax.yaxis.label.set_color(DARK)


def save_coin_plot():
    data = np.loadtxt(OUTPUT / "coin_running.dat")
    n = data[:, 0]
    fraction = data[:, 1]

    fig, ax = plt.subplots()
    ax.plot(n, fraction, lw=1.2, color=BLUE, label="Simulated fraction")
    ax.axhline(0.5, color=RED, ls="--", lw=1.5, label="Expected value")
    ax.set_xscale("log")
    ax.set_xlabel("Number of tosses")
    ax.set_ylabel("Fraction of heads")
    ax.set_title("Coin Tosses and the Law of Large Numbers")
    ax.legend(loc="best")
    polish_axes(ax)
    plt.tight_layout()
    plt.savefig(FIGURES / "exercise_1_coin_tosses.png", dpi=160)
    plt.close()


def save_pi_plot():
    data = np.loadtxt(OUTPUT / "pi_results.dat")
    n = data[:, 0]
    pi_hat = data[:, 1]
    error = data[:, 2]
    reference = error[0] * np.sqrt(n[0] / n)

    fig, ax = plt.subplots()
    ax.loglog(n, error, "o-", color=GREEN, lw=2.0, ms=6, label="Absolute error")
    ax.loglog(n, reference, "--", color=GOLD, lw=1.8, label=r"$N^{-1/2}$ reference")
    ax.set_xlabel("Sample size N")
    ax.set_ylabel(r"Absolute error $|\hat{\pi}-\pi|$")
    ax.set_title(r"Monte Carlo Estimate of $\pi$")
    ax.legend(loc="best")
    polish_axes(ax)
    plt.tight_layout()
    plt.savefig(FIGURES / "exercise_2_pi_error.png", dpi=160)
    plt.close()

    fig, ax = plt.subplots()
    ax.semilogx(n, pi_hat, "o-", color=BLUE, lw=2.0, ms=6, label=r"Monte Carlo estimate $\hat{\pi}$")
    ax.axhline(np.pi, color=RED, ls="--", lw=1.8, label=r"True value $\pi$")
    ax.set_xlabel("Sample size N")
    ax.set_ylabel(r"Estimated value of $\pi$")
    ax.set_title(r"Monte Carlo Approximation of $\pi$")
    ax.legend(loc="best")
    polish_axes(ax)
    plt.tight_layout()
    plt.savefig(FIGURES / "exercise_2_pi_estimate.png", dpi=160)
    plt.close()


def save_y_square_plot():
    y = np.loadtxt(OUTPUT / "y_square_sample.dat")
    grid = np.linspace(1e-4, 1.0, 800)
    pdf = 1.0 / (2.0 * np.sqrt(grid))

    fig, ax = plt.subplots()
    ax.hist(
        y,
        bins=80,
        density=True,
        color="#8FB6D8",
        edgecolor="#315A7C",
        linewidth=0.35,
        alpha=0.9,
        label="Simulated density",
    )
    ax.plot(grid, pdf, color=RED, lw=2.2, label=r"$f_Y(y)=1/(2\sqrt{y})$")
    ax.set_xlabel("y")
    ax.set_ylabel("Probability density")
    ax.set_title(r"Change of Variables: $Y=U^2$")
    ax.set_ylim(0, 8)
    ax.legend(loc="best")
    polish_axes(ax)
    plt.tight_layout()
    plt.savefig(FIGURES / "exercise_3_change_of_variables.png", dpi=160)
    plt.close()


def save_exponential_pdf_plot():
    y = np.loadtxt(OUTPUT / "exponential_sample.dat")
    lam = 1.5
    grid = np.linspace(0.0, np.quantile(y, 0.995), 800)
    pdf = lam * np.exp(-lam * grid)

    fig, ax = plt.subplots()
    ax.hist(
        y,
        bins=90,
        density=True,
        color="#A8CFA3",
        edgecolor="#3E6B3B",
        linewidth=0.35,
        alpha=0.9,
        label="Simulated density",
    )
    ax.plot(grid, pdf, color=RED, lw=2.2, label=r"$\lambda e^{-\lambda y}$")
    ax.set_xlabel("y")
    ax.set_ylabel("Probability density")
    ax.set_title(r"Inverse Transform Sampling, $\lambda=1.5$")
    ax.legend(loc="best")
    polish_axes(ax)
    plt.tight_layout()
    plt.savefig(FIGURES / "exercise_4_exponential_pdf.png", dpi=160)
    plt.close()


def save_empirical_cdf_plot():
    data = np.loadtxt(OUTPUT / "empirical_cdf.dat")
    y = data[:, 0]
    empirical = data[:, 1]
    lam = 1.5
    grid = np.linspace(0.0, np.quantile(y, 0.995), 800)
    exact = 1.0 - np.exp(-lam * grid)

    fig, ax = plt.subplots()
    ax.step(y, empirical, where="post", lw=1.1, color=BLUE, label="Empirical CDF")
    ax.plot(grid, exact, color=RED, lw=2.0, ls="--", label=r"Exact CDF")
    ax.set_xlabel("y")
    ax.set_ylabel("Cumulative probability")
    ax.set_title("Empirical CDF Compared with the Exact CDF")
    ax.set_xlim(0, grid[-1])
    ax.set_ylim(0, 1.02)
    ax.legend(loc="lower right")
    polish_axes(ax)
    plt.tight_layout()
    plt.savefig(FIGURES / "exercise_5_empirical_cdf.png", dpi=160)
    plt.close()


def main():
    save_coin_plot()
    save_pi_plot()
    save_y_square_plot()
    save_exponential_pdf_plot()
    save_empirical_cdf_plot()
    print(f"Saved plots in {FIGURES.resolve()}")


if __name__ == "__main__":
    main()
