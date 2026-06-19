import json
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from pathlib import Path
from datetime import datetime


##########
# VISUALIZATION of SUMMARY TRAINING
##########

class ExperimentVisualizer:

    def __init__(self, experiment_path, create_summary=True):
        self.exp_path = Path(experiment_path)
        self.raw_path = self.exp_path / "output"
        self.summary_path = self.exp_path / "summary"

        if create_summary:
            self.summary_path.mkdir(exist_ok=True)
            (self.summary_path / "rmse").mkdir(exist_ok=True)
            (self.summary_path / "relative_error").mkdir(exist_ok=True)
            self.raw_path.mkdir(exist_ok=True)

    def create_training_summary(self, history, config=None, zoomed=False):
        # macht training plots
        fig = plt.figure(figsize=(12, 8))
        gs = gridspec.GridSpec(2, 2, figure=fig)

        ax1 = fig.add_subplot(gs[0, :])
        if zoomed:
            start_epoch = max(5, len(history.history['loss']) // 10)
            train_loss = history.history['loss'][start_epoch:]
            val_loss = history.history['val_loss'][start_epoch:] if 'val_loss' in history.history else None
            epochs = list(range(start_epoch, len(history.history['loss'])))
            title_suffix = ' (Zoomed - Epochs {}-{})'.format(start_epoch, len(history.history['loss']) - 1)
        else:
            train_loss = history.history['loss']
            val_loss = history.history['val_loss'] if 'val_loss' in history.history else None
            epochs = list(range(len(train_loss)))
            title_suffix = ''

        ax1.plot(epochs, train_loss, label='Train Loss', linewidth=2)
        if val_loss is not None:
            ax1.plot(epochs, val_loss, label='Val Loss', linewidth=2)
        ax1.set_xlabel('Epoch')
        ax1.set_ylabel('Loss')
        ax1.set_title('Training' + title_suffix)
        ax1.legend()
        ax1.grid(True, alpha=0.3)

        if zoomed and len(train_loss) > 0:
            y_min = 0
            y_max = max(max(train_loss), max(val_loss) if val_loss else 0) * 1.1
            ax1.set_ylim([y_min, y_max])

        if 'out_rho_loss' in history.history:
            ax2 = fig.add_subplot(gs[1, 0])

            if zoomed:
                rho_loss = history.history['out_rho_loss'][start_epoch:]
                val_rho_loss = history.history['val_out_rho_loss'][
                    start_epoch:] if 'val_out_rho_loss' in history.history else None
            else:
                rho_loss = history.history['out_rho_loss']
                val_rho_loss = history.history['val_out_rho_loss'] if 'val_out_rho_loss' in history.history else None

            ax2.plot(epochs, rho_loss, label='Rho Loss', color='blue')
            if val_rho_loss is not None:
                ax2.plot(epochs, val_rho_loss, label='Val Rho', color='blue', linestyle='--')
            ax2.set_xlabel('Epoch')
            ax2.set_ylabel('Rho Loss')
            ax2.set_title('Density Prediction')
            ax2.legend()
            ax2.grid(True, alpha=0.3)

            if zoomed and len(rho_loss) > 0:
                y_min = 0
                y_max = max(max(rho_loss), max(val_rho_loss) if val_rho_loss else 0) * 1.1
                ax2.set_ylim([y_min, y_max])

            ax3 = fig.add_subplot(gs[1, 1])

            if zoomed:
                v_loss = history.history['out_v_loss'][start_epoch:]
                val_v_loss = history.history['val_out_v_loss'][
                    start_epoch:] if 'val_out_v_loss' in history.history else None
            else:
                v_loss = history.history['out_v_loss']
                val_v_loss = history.history['val_out_v_loss'] if 'val_out_v_loss' in history.history else None

            ax3.plot(epochs, v_loss, label='Velocity Loss', color='green')
            if val_v_loss is not None:
                ax3.plot(epochs, val_v_loss, label='Val Velocity', color='green', linestyle='--')
            ax3.set_xlabel('Epoch')
            ax3.set_ylabel('Velocity Loss')
            ax3.set_title('Velocity Prediction')
            ax3.legend()
            ax3.grid(True, alpha=0.3)

            if zoomed and len(v_loss) > 0:
                y_min = 0
                y_max = max(max(v_loss), max(val_v_loss) if val_v_loss else 0) * 1.1
                ax3.set_ylim([y_min, y_max])

        if config:
            fig.suptitle(f"Experiment: {config.get('experiment_id', 'Unknown')}\n"
                         f"LR: {config['hyperparameters']['learning_rate']}, "
                         f"Loss Weights: {config['hyperparameters']['loss_weights']}, "
                         f"Feats: {config['hyperparameters']['feats']}")

        plt.tight_layout()
        return fig

    def create_prediction_summary(self, predictions, labels, timesteps_to_show=None, metric_type="rmse"):
        # zeigt predictions

        if timesteps_to_show is None:
            total_steps = min(predictions[0].shape[1], labels[0].shape[1])
            if total_steps >= 5:
                timesteps_to_show = [0, total_steps // 4, total_steps // 2, 3 * total_steps // 4, total_steps - 1]
            else:
                timesteps_to_show = list(range(total_steps))

        n_samples = min(3, predictions[0].shape[0])
        n_times = len(timesteps_to_show)

        fig, axes = plt.subplots(n_samples * 2, n_times, figsize=(n_times * 3, n_samples * 6))

        if n_samples == 1:
            axes = axes.reshape(2, -1)

        for sample_idx in range(n_samples):
            for t_idx, timestep in enumerate(timesteps_to_show):
                ax_rho = axes[sample_idx * 2, t_idx]

                if t_idx == 0:
                    ax_rho.set_ylabel(f'Sample {sample_idx + 1}\nDensity', fontsize=10)

                label_rho = labels[0][sample_idx, timestep, 0]
                pred_rho = predictions[0][sample_idx, timestep, 0]

                if metric_type == "rmse":
                    diff = ax_rho.imshow(pred_rho - label_rho, cmap='RdBu_r', vmin=-0.1, vmax=0.1)
                    rmse_value = np.sqrt(np.mean((pred_rho - label_rho) ** 2))
                    if sample_idx == 0:
                        ax_rho.set_title(f'Density RMSE\nt={timestep}, Value={rmse_value:.3f}')
                    else:
                        ax_rho.set_title(f't={timestep}, RMSE={rmse_value:.3f}')
                else:
                    mean_val = np.mean(np.abs(label_rho)) + 1e-8
                    rel_error = (pred_rho - label_rho) / mean_val
                    diff = ax_rho.imshow(rel_error, cmap='RdBu_r', vmin=-1.0, vmax=1.0)
                    rel_error_value = np.sqrt(np.mean(rel_error ** 2))
                    if sample_idx == 0:
                        ax_rho.set_title(f'Density Relative Error\nt={timestep}, Value={rel_error_value:.1%}')
                    else:
                        ax_rho.set_title(f't={timestep}, Rel.Err={rel_error_value:.1%}')

                plt.colorbar(diff, ax=ax_rho, fraction=0.046)

                ax_v = axes[sample_idx * 2 + 1, t_idx]

                if t_idx == 0:
                    ax_v.set_ylabel(f'Sample {sample_idx + 1}\nVelocity', fontsize=10)

                label_v_mag = np.sqrt(labels[1][sample_idx, timestep, 0] ** 2 +
                                      labels[1][sample_idx, timestep, 1] ** 2)
                pred_v_mag = np.sqrt(predictions[1][sample_idx, timestep, 0] ** 2 +
                                     predictions[1][sample_idx, timestep, 1] ** 2)

                if metric_type == "rmse":
                    diff_v = ax_v.imshow(pred_v_mag - label_v_mag, cmap='RdBu_r', vmin=-0.1, vmax=0.1)
                    v_rmse = np.sqrt(np.mean((pred_v_mag - label_v_mag) ** 2))
                    if sample_idx == 0:
                        ax_v.set_title(f'Velocity Magnitude RMSE\nt={timestep}, Value={v_rmse:.3f}')
                    else:
                        ax_v.set_title(f't={timestep}, RMSE={v_rmse:.3f}')
                else:
                    mean_v_val = np.mean(np.abs(label_v_mag)) + 1e-8
                    v_rel_error = (pred_v_mag - label_v_mag) / mean_v_val
                    diff_v = ax_v.imshow(v_rel_error, cmap='RdBu_r', vmin=-1.0, vmax=1.0)
                    v_rel_error_value = np.sqrt(np.mean(v_rel_error ** 2))
                    if sample_idx == 0:
                        ax_v.set_title(f'Velocity Relative Error\nt={timestep}, Value={v_rel_error_value:.1%}')
                    else:
                        ax_v.set_title(f't={timestep}, Rel.Err={v_rel_error_value:.1%}')

                plt.colorbar(diff_v, ax=ax_v, fraction=0.046)

        # Titel anpassen basierend auf Metrik
        if metric_type == "rmse":
            plt.suptitle(
                'Prediction RMSE Visualization (Prediction - Ground Truth)\nBlue: Underestimation | Red: Overestimation',
                fontsize=12)
        else:
            plt.suptitle('Prediction Relative Error Visualization\nBlue: Underestimation | Red: Overestimation (±100%)',
                         fontsize=12)

        plt.tight_layout()
        return fig

    def create_error_evolution(self, predictions, labels, metric_type="rmse"):
        # fehler ueber zeit
        n_timesteps = predictions[0].shape[1]

        rho_errors = []
        v_errors = []

        for t in range(n_timesteps):
            # rho
            rho_diff = predictions[0][:, t, 0] - labels[0][:, t, 0]

            # velocity
            v_pred = predictions[1][:, t, :]
            v_label = labels[1][:, t, :]
            v_mag_pred = np.sqrt(v_pred[:, :, :, 0] ** 2 + v_pred[:, :, :, 1] ** 2)
            v_mag_label = np.sqrt(v_label[:, :, :, 0] ** 2 + v_label[:, :, :, 1] ** 2)

            if metric_type == "rmse":
                # RMSE Berechnung
                rho_errors.append(np.sqrt(np.mean(rho_diff ** 2)))
                v_errors.append(np.sqrt(np.mean((v_mag_pred - v_mag_label) ** 2)))
                ylabel = "RMSE"
                title_suffix = "RMSE"
            else:
                # Relative Error Berechnung
                rho_mean = np.mean(np.abs(labels[0][:, t, 0])) + 1e-8
                v_mean = np.mean(np.abs(v_mag_label)) + 1e-8
                rho_errors.append(np.sqrt(np.mean((rho_diff / rho_mean) ** 2)) * 100)  # In Prozent
                v_errors.append(np.sqrt(np.mean(((v_mag_pred - v_mag_label) / v_mean) ** 2)) * 100)
                ylabel = "Relative Error (%)"
                title_suffix = "Relative Error"

        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

        ax1.plot(rho_errors, linewidth=2, color='blue')
        ax1.set_xlabel('Timestep')
        ax1.set_ylabel(ylabel)
        ax1.set_title(f'Density Prediction {title_suffix} Over Time')
        ax1.grid(True, alpha=0.3)
        # Markiere Start der echten Vorhersagen
        ax1.axvline(x=5, color='red', linestyle='--', alpha=0.5, label='Start of Predictions')
        ax1.text(5, ax1.get_ylim()[1] * 0.9, ' Context → Prediction', fontsize=9, color='red')
        ax1.legend(loc='upper right')

        ax2.plot(v_errors, linewidth=2, color='green')
        ax2.set_xlabel('Timestep')
        ax2.set_ylabel(ylabel)
        ax2.set_title(f'Velocity Prediction {title_suffix} Over Time')
        ax2.grid(True, alpha=0.3)
        # Markiere Start der echten Vorhersagen
        ax2.axvline(x=5, color='red', linestyle='--', alpha=0.5, label='Start of Predictions')
        ax2.text(5, ax2.get_ylim()[1] * 0.9, ' Context → Prediction', fontsize=9, color='red')
        ax2.legend(loc='upper right')

        plt.tight_layout()
        return fig

    def save_metrics(self, history, predictions=None, labels=None):
        # metriken speichern
        metrics = {
            'timestamp': datetime.now().isoformat(),
            'final_metrics': {
                'train_loss': float(history.history['loss'][-1]),
                'best_train_loss': float(min(history.history['loss'])),
            }
        }

        if 'val_loss' in history.history:
            metrics['final_metrics']['val_loss'] = float(history.history['val_loss'][-1])
            metrics['final_metrics']['best_val_loss'] = float(min(history.history['val_loss']))
            metrics['final_metrics']['best_epoch'] = int(np.argmin(history.history['val_loss']) + 1)

        # metriken auf test-daten
        if predictions is not None and labels is not None:
            # Rho (Density) Metriken
            rho_diff = predictions[0] - labels[0]
            rho_rmse = float(np.sqrt(np.mean(rho_diff ** 2)))
            rho_mae = float(np.mean(np.abs(rho_diff)))

            # Velocity Metriken
            v_pred = predictions[1]
            v_label = labels[1]
            v_diff = v_pred - v_label
            v_rmse = float(np.sqrt(np.mean(v_diff ** 2)))
            v_mae = float(np.mean(np.abs(v_diff)))

            # Velocity Magnitude Metriken (wie in error_evolution)
            v_mag_pred = np.sqrt(v_pred[..., 0] ** 2 + v_pred[..., 1] ** 2)
            v_mag_label = np.sqrt(v_label[..., 0] ** 2 + v_label[..., 1] ** 2)
            v_mag_diff = v_mag_pred - v_mag_label
            v_mag_rmse = float(np.sqrt(np.mean(v_mag_diff ** 2)))

            metrics['evaluation_metrics'] = {
                'dataset': 'test',  # Klarstellen dass es Test-Daten sind
                'density': {
                    'rmse': rho_rmse,
                    'mae': rho_mae,
                    'relative_error': rho_rmse / (np.mean(np.abs(labels[0])) + 1e-8)  # Relativer Fehler
                },
                'velocity': {
                    'rmse': v_rmse,
                    'mae': v_mae,
                    'magnitude_rmse': v_mag_rmse,
                    'relative_error': v_rmse / (np.mean(np.abs(labels[1])) + 1e-8)
                }
            }

        # History für Plots
        metrics['history'] = {
            'epochs': list(range(1, len(history.history['loss']) + 1)),
            'train_loss': [float(x) for x in history.history['loss']]
        }

        if 'val_loss' in history.history:
            metrics['history']['val_loss'] = [float(x) for x in history.history['val_loss']]

        return metrics

def create_experiment_summary(experiment_path, history=None, dataset=None, config=None):
    # create all plots at once
    visualizer = ExperimentVisualizer(experiment_path)

    # save config JSON
    config_src = Path(experiment_path) / "config.json"
    config_dst = visualizer.summary_path / "config.json"
    if config_src.exists() and not config_dst.exists():
        import shutil
        shutil.move(str(config_src), str(config_dst))
    elif config:
        with open(visualizer.summary_path / "config.json", 'w') as f:
            json.dump(config, f, indent=2)

    # Training Summary
    if history:
        # normal
        fig = visualizer.create_training_summary(history, config, zoomed=False)
        fig.savefig(visualizer.summary_path / "training_curves.png", dpi=100, bbox_inches='tight')
        plt.close(fig)

        # zoomed
        fig = visualizer.create_training_summary(history, config, zoomed=True)
        fig.savefig(visualizer.summary_path / "training_curves_zoomed.png", dpi=100, bbox_inches='tight')
        plt.close(fig)

    # visualizations
    for metric_type in ["rmse", "relative_error"]:
        output_dir = visualizer.summary_path / metric_type

        # Prediction Summary und Error Evolution (unterschiedlich je nach Metrik)
        if dataset and hasattr(dataset, 'predictions'):
            fig = visualizer.create_prediction_summary(dataset.predictions, dataset.labels,
                                                       metric_type=metric_type)
            fig.savefig(output_dir / "prediction_samples.png", dpi=100, bbox_inches='tight')
            plt.close(fig)

            fig = visualizer.create_error_evolution(dataset.predictions, dataset.labels,
                                                    metric_type=metric_type)
            fig.savefig(output_dir / "error_evolution.png", dpi=100, bbox_inches='tight')
            plt.close(fig)

    # save metrics
    if history:
        metrics = visualizer.save_metrics(
            history,
            dataset.predictions if dataset and hasattr(dataset, 'predictions') else None,
            dataset.labels if dataset and hasattr(dataset, 'labels') else None
        )

        with open(visualizer.summary_path / "metrics.json", 'w') as f:
            json.dump(metrics, f, indent=2)

    print(f"summary in: {visualizer.summary_path}")
    print(f"  - rmse: {visualizer.summary_path}/rmse/")
    print(f"  - relative error: {visualizer.summary_path}/relative_error/")
    return visualizer.summary_path
