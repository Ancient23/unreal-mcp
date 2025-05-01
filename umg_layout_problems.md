# Problems with Current WBP_PricingPage Layout

Based on the initial component setup in `WBP_PricingPage` compared to the target design, the following issues need addressing:

1.  **Root Container Placement & Sizing:**
    *   The main content (`MainVBox`) resides within the default `CanvasPanel` at the top-left (0,0).
    *   The layout needs proper positioning (e.g., centering, margins) within the viewport.
    *   The overall size of the pricing section is undefined.

2.  **Missing Layout Spacing (Padding/Margins):**
    *   No visual space exists between major sections (Title, Subtitle, Toggle, Cards Box).
    *   No visual space exists between the pricing cards (`HobbyBorder`, `ProBorder`, `BusinessBorder`) within the `CardsBox`.
    *   No internal padding exists within the pricing cards, causing content (text, buttons) to touch the edges and each other.

3.  **Lack of Styling:**
    *   All widgets use default UMG styles (colors, fonts, button shapes, border appearances).
    *   Requires implementation of the dark theme, specific text styles (color, font size/weight), button styling (background color, text color, rounded corners for toggle), border styling (background color, rounded corners), and the gradient for the Pro card.

4.  **Incomplete Card Content Structure:**
    *   Feature list items are just `TextBlock`s.
    *   Need to implement the structure for features (e.g., `HorizontalBox` containing checkmark `Image` + feature `TextBlock`).
    *   Info icons (`Image` or `Button`+`Image`) are missing from the Pro card features.

5.  **Incorrect Sizing & Alignment:**
    *   Elements within containers may not be sized or aligned correctly (e.g., text within buttons, price/suffix alignment).
    *   The three pricing cards likely don't have appropriate or equal sizing within their parent `HorizontalBox` (`CardsBox`). 