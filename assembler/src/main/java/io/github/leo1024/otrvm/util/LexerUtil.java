package io.github.leo1024.otrvm.util;

import io.github.leo1024.otrvm.conf.TokenKind;
import io.github.leo1024.otrvm.ex.TokenizerException;
import io.github.leo1024.otrvm.lexer.Token;

import java.util.regex.Pattern;

/**
 * Lexical analysis utility class providing common token recognition patterns and methods.
 */
public class LexerUtil {
    // Pattern for matching decimal integers (one or more digits)
    static final Pattern INT_PATTERN = Pattern.compile("[0-9]+");

    // Pattern for matching floating-point numbers (optional integer part + decimal point + mandatory fractional part)
    // Note: Current pattern doesn't match multi-digit integer parts (e.g., "123.45"). Consider: [0-9]*\\.[0-9]+
    static final Pattern FLOAT_PATTERN = Pattern.compile("[0-9]?\\.[0-9]+");

    // Pattern for hexadecimal literals (# prefix + one or more hex digits)
    static final Pattern HEX_PATTERN = Pattern.compile("#[0-9a-fA-F]+");

    // Pattern for reference variables ($ prefix + valid Java identifier)
    // Rules: First character must be letter/_/$, subsequent characters can be letters/numbers/_/$
    static final Pattern REF_VAR_PATTERN = Pattern.compile("^\\$[a-zA-Z_$][\\w$]*");

    /**
     * <p>Creates a numeric token (either integer or float) from input text.</p>
     * <p>Processing logic:</p>
     *
     * <li>First checks for float pattern (avoids misinterpreting "0.5" as integer "0" and dot)</li>
     * <li>Then checks integer pattern</li>
     * <li> Throws exception if neither pattern matches</li>
     *
     * @param line   Line number for error reporting
     * @param column Column number for error reporting
     * @param text   Input text to analyze
     * @return Token of appropriate numeric type
     * @throws TokenizerException When text doesn't match any numeric format
     */
    public static Token createDigitToken(int line, int column, String text) {
        // Prioritize float check (must contain decimal point)
        boolean isFloat = FLOAT_PATTERN.matcher(text).matches();
        if (isFloat) {
            return new Token(TokenKind.FLOAT, text, line, column);
        }

        // Check for integer pattern (digit sequence)
        boolean isInt = INT_PATTERN.matcher(text).matches();
        if (isInt) {
            return new Token(TokenKind.INTEGER, text, line, column);
        }

        // Handle non-numeric input
        throw TokenizerException.create(line, column, "Invalid digit token", text);
    }

    /**
     * Validates hexadecimal literal format.
     *
     * @param hex String to validate
     * @return Validation result (true = valid, false = invalid)
     * <p>
     * Note: Prints error to stderr on failure (suitable for debugging).
     * Production systems should use proper logging.
     */
    public static boolean checkHexLiteral(String hex) {
        boolean match = HEX_PATTERN.matcher(hex).matches();
        if (!match) {
            System.err.printf("Invalid hex literal:%s%n", hex);
        }
        return match;
    }

    /**
     * Validates reference variable format ($-prefixed identifier).
     *
     * <pre>
     * <code>
     *  // Valid examples:
     *  $count, $_value, $$var
     *  // Invalid examples:
     *  $1var (starts with digit), myVar (missing $)
     * </code>
     * </pre>
     *
     * @param variable Variable name to validate
     * @return Validation result (true = valid, false = invalid)
     */
    public static boolean checkRefVar(String variable) {
        boolean match = REF_VAR_PATTERN.matcher(variable).matches();
        if (!match) {
            System.err.printf("Invalid ref var:%s%n", variable);
        }
        return match;
    }
}