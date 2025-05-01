# UMG Components for Pricing Page Layout

Here's a list of Unreal Engine UMG Widget components suitable for recreating the provided pricing page design:

1.  **`CanvasPanel` (Root)**:
    *   **Explanation**: Often the root panel for UMG widgets, allowing absolute positioning of child elements. Alternatively, a `VerticalBox` could serve as the root if absolute positioning isn't strictly needed for the main sections.

2.  **`VerticalBox` (Main Layout)**:
    *   **Explanation**: To stack the main sections vertically: Title, Subtitle, Toggle Buttons, and the row of Pricing Cards.

3.  **`TextBlock` (Titles & Text)**:
    *   **Explanation**: Used for all text elements: "Pricing", "Choose the plan...", plan names ("Hobby", "Pro", "Business"), prices ("Free", "$20", "$40"), price suffixes ("/month"), section headers ("Includes", "Everything in..."), and feature list descriptions. Font, size, and color can be customized.

4.  **`HorizontalBox` (Button Toggle & Card Row)**:
    *   **Explanation**:
        *   One `HorizontalBox` to hold the "Monthly" and "Yearly" toggle buttons side-by-side.
        *   Another `HorizontalBox` within the main `VerticalBox` to arrange the three pricing plan cards horizontally.

5.  **`Button` (Toggles & CTAs)**:
    *   **Explanation**: Used for the "Monthly" / "Yearly" toggles and the call-to-action buttons ("DOWNLOAD", "OTHERS", "GET STARTED"). Styling will be needed to match the appearance (background color, text style, rounded corners for the toggle).

6.  **`Border` (Card Background/Frame)**:
    *   **Explanation**: Used as the container for each pricing card ("Hobby", "Pro", "Business"). Provides the background color (including the gradient for the "Pro" card via its Brush property) and the rounded corner visual style.

7.  **`VerticalBox` (Inside Cards)**:
    *   **Explanation**: Placed inside each `Border` (pricing card) to stack the card's content vertically (Plan Title, Price info, Includes section, Features, Button).

8.  **`HorizontalBox` (Price & Suffix / Feature Items / Hobby Buttons)**:
    *   **Explanation**:
        *   Inside cards: To place the Price `TextBlock` and Suffix `TextBlock` side-by-side.
        *   Inside cards: For each feature list item, to place the checkmark `Image`/`TextBlock` next to the feature description `TextBlock`.
        *   Inside Hobby card: To place the "DOWNLOAD" and "OTHERS" buttons side-by-side.

9.  **`Image` (Checkmarks & Info Icons)**:
    *   **Explanation**: Used for the checkmark icons in the feature lists and potentially the circled question mark info icons (if they are static images). If the info icons need tooltips on hover, a `Button` containing an `Image` might be better.

10. **`Spacer` or Padding**:
    *   **Explanation**: Used within `HorizontalBox` and `VerticalBox` containers to create visual spacing between elements (e.g., between pricing cards, between text lines, around buttons). Padding can often be set directly on container slots as well.

11. **`SizeBox` (Optional)**:
    *   **Explanation**: Can wrap elements or be used within containers to enforce specific widths or heights, helping maintain consistent card sizes or element dimensions. 