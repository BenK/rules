<?php
// $Id$

/**
 * @file Contains UI for diverse plugins provided by Rules.
 */

/**
 * Rule specific UI.
 */
class RulesRuleUI extends RulesActionContainerUI {

  protected $rule, $conditions;

  public function __construct(FacesExtendable $object) {
    parent::__construct($object);
    $this->rule = $object;
    // Make sure the condition is intialized and then get it.
    $this->rule->conditions();
    $this->conditions = $this->property('conditions');
  }

  public function form(&$form, &$form_state, $options = array()) {
    $form += array('conditions' => array('#weight' => -5));
    $this->conditions->form($form['conditions'], $form_state);

    // Add actions form.
    $iterator = new RecursiveIteratorIterator($this->rule->actions(), RecursiveIteratorIterator::SELF_FIRST);
    parent::form($form, $form_state, $options, $iterator);
  }

  /**
   * Applies the values of the form to the rule configuration.
   */
  function form_extract_values($form, $form_state) {
    // Run condition and action container value extraction.
    $this->conditions->extender('RulesConditionContainerUI')->form_extract_values($form['conditions'], $form_state);
    parent::form_extract_values($form, $form_state);
  }


  public function operations() {
    // When rules are listed only show the edit and delete operations.
    $ops = parent::operations();
    $ops['#links'] = array_intersect_key($ops['#links'], array_flip(array('edit', 'delete')));
    return $ops;
  }
}

/**
 * Reaction rule specific UI.
 */
class RulesReactionRuleUI extends RulesRuleUI {

  public function form(&$form, &$form_state, $options = array()) {

    $form['elements']['events'] = array(
      '#type' => 'container',
      '#title' => t('On'),
    );

    $event_info = rules_fetch_data('event_info');
    foreach($this->rule->events() as $event_name) {
      $form['elements']['events'][$event_name] = array(
        '#markup' => check_plain($event_info[$event_name]['label']),
        '#prefix' => '<div>',
        '#suffix' => '&nbsp;<span class="rules_rule_event">' . l(t('delete'), RULES_UI_RULE_PATH) . '</span></div>',
      );
    }

    $form['elements']['events']['add'] = array(
      '#markup' => t(l('+ Add event', RULES_UI_RULE_PATH . '/' . $this->rule->name . '/add/event')),
    );

    parent::form($form, $form_state, $options);
  }
}

/**
 * Rule set specific UI.
 */
class RulesRuleSetUI extends RulesActionContainerUI {

  public function form(&$form, &$form_state, $options = array(), $iterator = NULL) {
    // Pass an iterator just iterating over the rules, thus no further child
    // elements will be displayed.
    parent::form($form, $form_state, $options, $this->element->getIterator());
    // Only show the add rule link.
    $form['add']['#links'] = array_intersect_key($form['add']['#links'], array('add_rule' => 1));
    $form['elements']['#attributes']['class'][] = 'rules-rule-set';
    $form['elements']['#heading'] = t('Rules');
  }

}


