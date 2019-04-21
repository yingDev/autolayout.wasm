Kiwi Release Notes
==================

Wrappers 1.0.2 | Solver 1.0.1 | unreleased
------------------------------------------
- prevent attempting a dual optimize on a dummy row PR #56 closes #15
- add ``dump`` and ``dumps`` methods to inspect the internal state of the
  solver PR #56
- test on Python 3.7 PR #51
- improvemnts to setup.py and tests PR #46 #50

Wrappers 1.0.1 | Solver 1.0.0 | 24/10/2017
------------------------------------------
- allow unicode strings for variable name in Python 2
- allow unicode strings as strength specifiers in Python 2

Wrappers 1.0.0 | Solver 1.0.0 | 09/06/2017
------------------------------------------
- Allow anonymous variables (solver PR #32, wrappers PR #22)
- Solver: Define binary operators as free functions (PR #23)
- Wrappers: support for Python 3 (PR #13)
- Wrappers: drop distribute dependency in favor of setuptools (PR #22)
- Wrappers: add a comprehensive test suite

Wrappers 0.1.3 | Solver 0.1.1 | 07/12/2013
------------------------------------------
- Update the build script to remove the need for build.py

Wrappers 0.1.2 | Solver 0.1.1 | 01/15/2013
------------------------------------------
- Fix issue #2. Bad handling of zero-size constraints.

Wrappers 0.1.1 | Solver 0.1.0 | 01/13/2013
------------------------------------------
- Initial public release.
