<?php
// $Id$

/**
 * @file Invokes events on behalf core modules. Usually this should be
 *   directly in the module providing rules integration instead.
 *
 * @addtogroup rules
 * @{
 */

/**
 * Implements hook_entity_load().
 *
 * We keep a clone of the unchanged entity, as loaded from the database in a
 * static variable. We update/create this static also on update/insert. That way
 * the static variable always represents the entity as currently saved.
 *
 * @see rules_entity_insert()
 * @see rules_entity_update()
 * @see rules_events_entity_unchanged()
 */
function rules_entity_load($entities, $type) {
  $unchanged = &drupal_static('rules_entity_unchanged', array());
  foreach ($entities as $id => $entity) {
    $unchanged[$type][$id] = clone $entity;
  }
}

/**
 * Implements hook_entity_insert().
 */
function rules_entity_insert($entity, $type) {
  $unchanged = &drupal_static('rules_entity_unchanged', array());
  list($id) = entity_extract_ids($type, $entity);
  $unchanged[$type][$id] = clone $entity;
  unset($unchanged[$type][$id]->is_new);
}

/**
 * Implements hook_entity_update().
 */
function rules_entity_update($entity, $type) {
  $unchanged = &drupal_static('rules_entity_unchanged', array());
  list ($id) = entity_extract_ids($type, $entity);
  $unchanged[$type][$id] = clone $entity;
}

/**
 * Gets an unchanged entity that doesn't contain any recent changes. This
 * handler assumes the name of the variable for the changed entity is the same
 * as for the unchanged entity but without the trailing "_unchanged"; e.g., for
 * the "node_unchanged" variable the handler assumes there is a "node" variable.
 *
 * Note that this handler only works properly if the entity has been loaded
 * with entity_load() before it is saved. Thus if a module serializes an
 * entity and saves it afterwards on another page load, the handler would fail.
 *
 * @see rules_entity_load()
 */
function rules_events_entity_unchanged($arguments, $name, $info) {
  $unchanged = &drupal_static('rules_entity_unchanged', array());
  // Cut of the trailing _unchanged.
  $var_name = substr($name, 0, -10);
  $type = $info['type'];
  list($id) = entity_extract_ids($type, $arguments[$var_name]);

  if (isset($id) && isset($unchanged[$type][$id])) {
    return $unchanged[$type][$id];
  }
  // Else try loading fresh from the db. This works only for 'presave' though.
  elseif (isset($id)) {
    $result = entity_load($type, array($id));
    return isset($result[$id]) ? $result[$id] : NULL;
  }
}


/**
 * Node events
 * For getting the unchanged node we currently have to implement a workaround.
 * If http://drupal.org/node/651240 gets in, we can simplify that.
 */

function rules_node_view($node, $view_mode) {
  rules_invoke_event('node_view', $node, $view_mode);
}

function rules_node_presave($node) {
  rules_invoke_event('node_presave', $node);
}

function rules_node_update($node) {
  rules_invoke_event('node_update', $node);
}

function rules_node_insert($node) {
  rules_invoke_event('node_insert', $node);
}

function rules_node_delete($node) {
  rules_invoke_event('node_delete', $node);
}



/**
 * Invoke user events.
 */

function rules_user_view($account, $view_mode) {
  rules_invoke_event('user_view', $account, $view_mode);
}

function rules_user_presave(&$edit, $account, $category) {
  if ($category == 'account') {
    rules_invoke_event('user_presave', $account);
  }
}

function rules_user_insert(&$edit, $account, $category) {
  if ($category == 'account') {
    rules_invoke_event('user_insert', $account);
  }
}

function rules_user_update(&$edit, $account, $category) {
  if ($category == 'account') {
    rules_invoke_event('user_update', $account);
  }
}

function rules_user_delete($account) {
  rules_invoke_event('user_delete', $account);
}

function rules_user_login(&$edit, $account) {
  rules_invoke_event('user_login', $account);
}

function rules_user_logout(&$edit, $account) {
  rules_invoke_event('user_logout', $account);
}

/**
 * Comment events.
 */

function rules_comment_view($comment) {
  rules_invoke_event('comment_view', $comment);
}

function rules_comment_presave($comment) {
  rules_invoke_event('comment_presave', $comment);
}

function rules_comment_update($comment) {
  rules_invoke_event('comment_update', $comment);
}

function rules_comment_insert($comment) {
  rules_invoke_event('comment_insert', $comment);
}

function rules_comment_delete($comment) {
  rules_invoke_event('comment_delete', $comment);
}


/**
 * Taxonomy events.
 */

function rules_taxonomy_term_update($taxonomy_term) {
  rules_invoke_event('taxonomy_term_update', $taxonomy_term);
}

function rules_taxonomy_term_presave($taxonomy_term) {
  rules_invoke_event('taxonomy_term_presave', $taxonomy_term);
}

function rules_taxonomy_term_insert($taxonomy_term) {
  rules_invoke_event('taxonomy_term_insert', $taxonomy_term);
}

function rules_taxonomy_term_delete($taxonomy_term) {
  rules_invoke_event('taxonomy_term_delete', $taxonomy_term);
}

function rules_taxonomy_vocabulary_update($taxonomy_vocabulary) {
  rules_invoke_event('taxonomy_vocabulary_update', $taxonomy_vocabulary);
}

function rules_taxonomy_vocabulary_presave($taxonomy_vocabulary) {
  rules_invoke_event('taxonomy_vocabulary_presave', $taxonomy_vocabulary);
}

function rules_taxonomy_vocabulary_insert($taxonomy_vocabulary) {
  rules_invoke_event('taxonomy_vocabulary_insert', $taxonomy_vocabulary);
}

function rules_taxonomy_vocabulary_delete($taxonomy_vocabulary) {
  rules_invoke_event('taxonomy_vocabulary_delete', $taxonomy_vocabulary);
}

/**
 * System events. Note that rules_init() is the main module file is used to
 * invoke the init event.
 */

function rules_cron() {
  rules_invoke_event('cron');
}

function rules_watchdog($log_entry) {
  rules_invoke_event('watchdog', $log_entry);
}

/**
 * Data actions access callback.
 * Returns TRUE if at least one type is available for configuring the action.
 */
function rules_data_action_access($type, $name) {
  if ($name == 'entity_fetch' || $name == 'entity_create' || $name == 'entity_query') {
    $types = array_keys(rules_data_action_type_options(NULL, $name));
    $op = $name == 'entity_create' ? 'create' : 'view';
  }
  elseif ($name == 'entity_save' || $name == 'entity_delete') {
    $cache = rules_get_cache();
    $types = $cache['action_info'][$name]['parameter']['data']['type'];
    $op = $name == 'entity_save' ? 'save' : 'delete';
  }
  foreach ($types as $key => $type) {
    if (!entity_metadata_entity_access($op, $type)) {
      unset($types[$key]);
    }
  }
  return !empty($types);
}

/**
 * Getter callback for the log entry message property.
 */
function rules_system_log_get_message($log_entry) {
  return t($log_entry['message'], (array)$log_entry['variables']);
}

/**
 * @}
 */