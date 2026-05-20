"""Generate true high-precision references and error tables for Homework 05.

The C executable writes the two double-precision experimental methods to
results/double_values.csv. This script then computes the reference sequence
with mpmath at 100 decimal digits and compares the double values against it.
The error calculations are performed in mpmath arithmetic, not by comparing
against a rounded double-precision reference.
"""

from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path

import mpmath as mp

RESULTS = Path("results")
INPUT = RESULTS / "double_values.csv"
ERRORS = RESULTS / "errors.csv"
SUMMARY = RESULTS / "summary.txt"
CHECKS = RESULTS / "verification_checks.csv"

L_MAX = 50
L_BACKWARD = 100
MP_DPS = 100
TINY = mp.mpf("1.0e-300")


def legendre_reference_mpmath(x_text: str, lmax: int = L_MAX) -> list[mp.mpf]:
    """Return P_0(x), ..., P_lmax(x) using 100-decimal-digit arithmetic."""
    x = mp.mpf(x_text)
    pref = [mp.mpf("0")] * (lmax + 1)
    pref[0] = mp.mpf("1")
    if lmax >= 1:
        pref[1] = x
    for ell in range(1, lmax):
        pref[ell + 1] = ((2 * ell + 1) * x * pref[ell] - ell * pref[ell - 1]) / (ell + 1)
    return pref


def relerr_mp(approx: mp.mpf, ref: mp.mpf) -> mp.mpf:
    denom = abs(ref) if abs(ref) > TINY else TINY
    return abs(approx - ref) / denom


def mp_sci(value: mp.mpf, digits: int = 17) -> str:
    """Return a compact, machine-readable scientific string."""
    return mp.nstr(value, n=digits, min_fixed=-6, max_fixed=6)


def read_double_values() -> dict[str, list[dict[str, str]]]:
    grouped: dict[str, list[dict[str, str]]] = defaultdict(list)
    with INPUT.open(newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            grouped[row["x"]].append(row)
    for rows in grouped.values():
        rows.sort(key=lambda r: int(r["l"]))
    return dict(sorted(grouped.items(), key=lambda item: float(item[0])))


def main() -> None:
    mp.mp.dps = MP_DPS
    RESULTS.mkdir(exist_ok=True)
    grouped = read_double_values()

    with ERRORS.open("w", newline="") as f_errors, CHECKS.open("w", newline="") as f_checks:
        error_fields = [
            "x",
            "l",
            "P_ref_100dps",
            "P_forward",
            "P_backward",
            "abs_err_forward",
            "rel_err_forward",
            "abs_err_backward",
            "rel_err_backward",
        ]
        check_fields = ["check", "value", "status"]
        writer = csv.DictWriter(f_errors, fieldnames=error_fields)
        check_writer = csv.DictWriter(f_checks, fieldnames=check_fields)
        writer.writeheader()
        check_writer.writeheader()

        summary_lines = [
            "Legendre polynomial stability homework",
            f"lmax = {L_MAX}",
            f"backward start L = {L_BACKWARD}",
            f"high-precision reference = mpmath with mp.dps = {MP_DPS} decimal digits",
            "errors are computed in mpmath arithmetic before being written to CSV",
            "",
        ]

        total_rows = 0
        for x_text, rows in grouped.items():
            pref_mp = legendre_reference_mpmath(x_text, L_MAX)
            max_abs_f = mp.mpf("0")
            max_rel_f = mp.mpf("0")
            max_abs_b = mp.mpf("0")
            max_rel_b = mp.mpf("0")
            backward_l0_error = mp.mpf("0")

            summary_lines.extend([f"x = {float(x_text):.2f}", "----------------------------------------"])

            for row in rows:
                ell = int(row["l"])
                pf = mp.mpf(row["P_forward"])
                pb = mp.mpf(row["P_backward"])
                pref = pref_mp[ell]

                abs_f = abs(pf - pref)
                rel_f = relerr_mp(pf, pref)
                abs_b = abs(pb - pref)
                rel_b = relerr_mp(pb, pref)

                max_abs_f = max(max_abs_f, abs_f)
                max_rel_f = max(max_rel_f, rel_f)
                max_abs_b = max(max_abs_b, abs_b)
                max_rel_b = max(max_rel_b, rel_b)
                if ell == 0:
                    backward_l0_error = abs(pb - mp.mpf("1"))

                writer.writerow(
                    {
                        "x": f"{float(x_text):.2f}",
                        "l": ell,
                        "P_ref_100dps": mp_sci(pref, 50),
                        "P_forward": mp_sci(pf, 17),
                        "P_backward": mp_sci(pb, 17),
                        "abs_err_forward": mp_sci(abs_f, 17),
                        "rel_err_forward": mp_sci(rel_f, 17),
                        "abs_err_backward": mp_sci(abs_b, 17),
                        "rel_err_backward": mp_sci(rel_b, 17),
                    }
                )
                total_rows += 1

            summary_lines.extend(
                [
                    f"max absolute error, forward  = {mp_sci(max_abs_f, 7)}",
                    f"max relative error, forward  = {mp_sci(max_rel_f, 7)}",
                    f"max absolute error, backward = {mp_sci(max_abs_b, 7)}",
                    f"max relative error, backward = {mp_sci(max_rel_b, 7)}",
                    "",
                ]
            )

            check_writer.writerow(
                {
                    "check": f"backward normalization P_back(0) at x={float(x_text):.2f}",
                    "value": mp_sci(backward_l0_error, 17),
                    "status": "PASS" if backward_l0_error < mp.mpf("1.0e-14") else "WARN",
                }
            )
            check_writer.writerow(
                {
                    "check": f"forward max relative error at x={float(x_text):.2f}",
                    "value": mp_sci(max_rel_f, 17),
                    "status": "PASS" if max_rel_f < mp.mpf("1.0e-12") else "WARN",
                }
            )
            check_writer.writerow(
                {
                    "check": f"backward max relative error at x={float(x_text):.2f}",
                    "value": mp_sci(max_rel_b, 17),
                    "status": "EXPECTED LARGE" if max_rel_b > mp.mpf("1.0e-3") else "UNEXPECTEDLY SMALL",
                }
            )

        expected_rows = 4 * (L_MAX + 1)
        check_writer.writerow(
            {
                "check": "number of rows in errors.csv",
                "value": str(total_rows),
                "status": "PASS" if total_rows == expected_rows else "FAIL",
            }
        )
        check_writer.writerow(
            {
                "check": "mpmath decimal precision",
                "value": str(MP_DPS),
                "status": "PASS" if MP_DPS >= 100 else "FAIL",
            }
        )

    summary_lines.extend(
        [
            "Observed behavior",
            "----------------------------------------",
            "For these ordinary Legendre polynomials, the forward recurrence remains accurate for the tested x values and l <= 50.",
            "The arbitrary backward sweep with Q_{L+1}=0, Q_L=1 and rescaling by Q_0 does not generally recover P_l(x).",
            "This is consistent with the fact that Miller's algorithm works when the wanted solution is minimal in the backward direction, which is not the case here.",
            "",
            "Verification",
            "----------------------------------------",
            "The reference values in errors.csv were computed with mpmath at mp.dps = 100, i.e. 100 decimal digits.",
            "The error calculations were also performed with mpmath arithmetic before conversion to output strings.",
            "The CSV file verification_checks.csv records row-count, normalization, precision, and expected-error checks.",
        ]
    )
    SUMMARY.write_text("\n".join(summary_lines) + "\n")


if __name__ == "__main__":
    main()
