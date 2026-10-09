# Contribution to OpenPisco

Before contributing, please review the following items:

1. You need to agree with the Contributor License Agreement detailed below.

2. Submitted code must adhere to [PEP 8](https://www.python.org/dev/peps/pep-0008/), except for the following deviations:
* Function Names: `CamelCase` (i.e. camel case starting with uppercase)
* Variables Names: `camelCase` (i.e. camel case starting with lowercase)
* No line limit length

3. Please read the contributing rules:
* All changes are integrated by IRT SystemX and Safran.
* Favor imports at the beginning of files.
* Each module must have a test function called `CheckIntegrity` that takes no argument and
returns the string `"ok"` if and only if the test was successful.
`CheckIntegrity` functions  must be local (as little imports as possible), aiming to test
only the functions defined in the file, as much as possible. All functions
in the file must be tested, if possible. Please limit the use of functions
from other file to reach that goal, and use small and simple data, otherwise
changes will be painful to propagate if many `CheckIntegrity` functions must be updated
as well. The `CheckIntegrity` functions also serve as examples and
client code for the library.
* Each module must have a main function calling `CheckIntegrity`.
* Coverage of the contributed code must be kept higher than 80%.
Please refer to the [README](README.md) of the project for information about the testing infrastructure.
* The documentation must adhere to the [Numpy docstring style](https://numpydoc.readthedocs.io/en/latest/format.html#docstring-standard).

## Contributor License Agreement

In order to clarify the intellectual property license granted with Contributions from any person or entity, IRT SystemX and Safran must have a Contributor License Agreement ("CLA") on file that has been signed by each Contributor, indicating agreement to the license terms below. This license is for your protection as a Contributor as well as the protection of IRT SystemX and Safran; it does not change your rights to use your own Contributions for any other purpose.
You accept and agree to the following terms and conditions for Your present and future Contributions submitted to IRT SystemX and Safran. Except for the license granted herein to IRT SystemX and Safran and recipients of software distributed by IRT SystemX and Safran, You reserve all right, title, and interest in and to Your Contributions.

1. Definitions.
* "You" (or "Your") shall mean the copyright owner or legal entity authorized by the copyright owner that is making this Agreement with IRT SystemX and Safran. For legal entities, the entity making a Contribution and all other entities that control, are controlled by, or are under common control with that entity are considered to be a single Contributor. For the purposes of this definition, "control" means (i) the power, direct or indirect, to cause the direction or management of such entity, whether by contract or otherwise, or (ii) ownership of fifty percent (50%) or more of the outstanding shares, or (iii) beneficial ownership of such entity.
* "Contribution" shall mean any original work of authorship, including any modifications or additions to an existing work, that is intentionally submitted by You to IRT SystemX and Safran for inclusion in, or documentation of, any of the products owned or managed by IRT SystemX and Safran (the "Work"). For the purposes of this definition, "submitted" means any form of electronic, verbal, or written communication sent to IRT SystemX and Safran or its representatives, including but not limited to communication on electronic mailing lists, source code control systems, and issue tracking systems that are managed by, or on behalf of, IRT SystemX and Safran for the purpose of discussing and improving the Work, but excluding communication that is conspicuously marked or otherwise designated in writing by You as "Not a Contribution."

2. Grant of Copyright License. Subject to the terms and conditions of this Agreement, You hereby grant to IRT SystemX and Safran and to recipients of software distributed by IRT SystemX and Safran a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable copyright license to reproduce, prepare derivative works of, publicly display, publicly perform, sublicense, and distribute Your Contributions and such derivative works.

3. Grant of Patent License. Subject to the terms and conditions of this Agreement, You hereby grant to IRT SystemX and Safran and to recipients of software distributed by IRT SystemX and Safran a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable (except as stated in this section) patent license to make, have made, use, offer to sell, sell, import, and otherwise transfer the Work, where such license applies only to those patent claims licensable by You that are necessarily infringed by Your Contribution(s) alone or by combination of Your Contribution(s) with the Work to which such Contribution(s) was submitted. If any entity institutes patent litigation against You or any other entity (including a cross-claim or counterclaim in a lawsuit) alleging that your Contribution, or the Work to which you have contributed, constitutes direct or contributory patent infringement, then any patent licenses granted to that entity under this Agreement for that Contribution or Work shall terminate as of the date such litigation is filed.

4. You represent that you are legally entitled to grant the above license. If your employer(s) has rights to intellectual property that you create that includes your Contributions, you represent that you have received permission to make Contributions on behalf of that employer, that your employer has waived such rights for your Contributions to IRT SystemX and Safran, or that your employer has executed a separate Corporate CLA with IRT SystemX and Safran.

5. You represent that each of Your Contributions is Your original creation (see section 7 for submissions on behalf of others). You represent that Your Contribution submissions include complete details of any third-party license or other restriction (including, but not limited to, related patents and trademarks) of which you are personally aware and which are associated with any part of Your Contributions.

6. You are not expected to provide support for Your Contributions, except to the extent You desire to provide support. You may provide support for free, for a fee, or not at all. Unless required by applicable law or agreed to in writing, You provide Your Contributions on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied, including, without limitation, any warranties or conditions of TITLE, NON- INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A PARTICULAR PURPOSE.

7. Should You wish to submit work that is not Your original creation, You may submit it to IRT SystemX and Safran separately from any Contribution, identifying the complete details of its source and of any license or other restriction (including, but not limited to, related patents, trademarks, and license agreements) of which you are personally aware, and conspicuously marking the work as "Submitted on behalf of a third-party: [named here]".

8. You agree to notify IRT SystemX and Safran of any facts or circumstances of which you become aware that would make these representations inaccurate in any respect.
