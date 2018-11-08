# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
- [ ] Enable packet managers
- [ ] Add badjes and descriptions
- [ ] Enable automatic documentation generation (doxygen??? doxygen!!!)
- [ ] Update readme file with correct content
- [x] Enable CodeCov to test project
- [x] Initialization of github project by required files and features
- [x] Version file

### added
- Enable LCov to perform code coverage and [CodeCov](https://codecov.io/) to represent results
- Emitter template as main part of signal module in [emitter.hpp](include/libmodules/emitter.hpp)
- Weak pointer implementation in [spy_pointer.hpp](include/libmodules/spy_pointer.hpp)
- Enable Travis CI to buil project by G++ 7 and run gtest

### fixed
- Swap neighboring elements in the linked list

### added
- Linked lis implementation

Directory structure
```
  libmodules
    CMakeList.txt
    include/
      libmodules/
        ...
    src
      cpp files
      ...
    example/
      cpp and hpp files
      ...
    test/
      cpp and hpp files
      ...
```
