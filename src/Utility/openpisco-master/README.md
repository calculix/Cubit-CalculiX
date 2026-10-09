# OpenPisco: A toolbox for topology optimization

## Licensing and copyright

Please consult the [LICENSE](LICENSE) file at the project root for licensing and copyright information.

## Notes for contributors

Contributors are welcome. Please read the [contribution guide](CONTRIBUTING.md) of the project before submitting contributions.

## Testing Infrastructure

Each module has a function called `CheckIntegrity` that takes no argument and returns the string `"ok"` if and only if the test was successful.

At the root of the source code repository and within each directory and subdirectory therein, there is an `__init__.py` script.
It has a variable named `_test` listing all submodules to be tested, used by the test infrastructure.
Submodules in this context refers to either a Python script or a subdirectory located in the current directory.

The `TestAll` function from the [muscat](https://gitlab.com/drti/muscat) library is used to test the library OpenPisco.
See documentation of this function for more details about the usage.

### Disabling tests

Some tests can be disabled using an environment variable.
A typical use case arises when a test relies on an external dependency that may not be available.

The feature relies on the definition of non-empty environment variables of the form:
```python
"appsname_NO_FAIL"
```

The function `SkipTest` from muscat is used to disable the test.
To disable the test the `CheckIntegrity` function should begin with the following line:
```python
if SkipTest("appsname_NO_FAIL"): return "ok"
```

See the documentation of `SkipTest` for more details about the usage.
