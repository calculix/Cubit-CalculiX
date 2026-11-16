Readable conditional code
=========================

Project-owned C++ uses explicit ``if`` / ``else`` blocks with braces instead of
conditional (ternary) operators. Apply this to runtime code, headers, commands,
GUI code and regression sources. Bundled third-party libraries retain their
upstream source style.

Place opening and closing block braces on separate lines (Allman style).

Prefer a named value and ordinary branch assignments::

   std::string operation;
   if (state == MaterialCardState::Active)
   {
       operation = " activate card ";
   }
   else
   {
       operation = " deactivate card ";
   }

Keep one statement per line. Preserve compound assignment operators and select
only the required branch. Branches that modify a selected object must modify
the original object, rather than a temporary copy. Keep references and const
initialization where they are part of the existing contract.

For initialization or an embedded expression whose evaluation must remain inside
a short-circuit condition, an immediately invoked lambda with explicit braced
branches preserves the evaluation point. Its explicit result type preserves
implicit numeric promotion and conversions between branch types. A
``std::common_type_t`` result is used where both operand types determine the
existing value type. Reference selections require a reference result or direct
branches that operate on the selected object.

Run the source-only check from the repository root::

   python tools/check_no_ternaries.py

The checker scans all project-owned C/C++ source and header extensions in
Framework, CalculiX, Postprocessor, Report and Solvers. It skips string and
character literals, raw strings, and comments. It does not configure or build
anything. Compilation and runtime tests remain separate validation steps.

Repository-wide cleanup
-----------------------

The cleanup replaced 1,080 parsed conditional expressions in 242 project-owned
files. The source checker reports zero remaining conditional operators across
2,055 project-owned C/C++ files. A parser comparison reported no additional
syntax errors, and a separate check confirmed that compound assignment operators
were retained. These source checks do not establish compilation or runtime
correctness; no build or runtime test was run during this cleanup.

Follow-up compiler diagnostics exposed pointer and const-reference errors in the
initial cleanup. A pointer declarator's ``*`` belongs to its declaration; branch
assignments must assign the pointer itself. Selections between mutable and const
objects must return a const reference when the consumer is read-only. These
cases were corrected in result readers, topology diagnostics, workspace graphics,
ID selection and drawing settings. The CSV quoting helper was also renamed to
``csv_quoted`` to avoid argument-dependent lookup selecting ``std::quoted`` when
streaming a mutable string to ``QTextStream``. These corrections were checked at
source level without rerunning the build.

A subsequent GUI build exposed additional pointer selections in both topology
preservation panels and temporary-value assignments in the Clip and Section
plane editors. These now use direct braced assignments to the selected pointer
or coordinate. The same audit corrected Postprocessor session loading to assign
color/deformation dataset references directly to the saved binding, rather than
to a temporary optional value. No build was run for these corrections.
